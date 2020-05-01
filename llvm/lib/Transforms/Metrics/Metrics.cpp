#include "llvm/InitializePasses.h"

#define DEBUG_TYPE "metrics"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/LoopInfo.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SCCIterator.h"

// TODO: add statistics
#include "llvm/ADT/Statistic.h"

#include "llvm/ADT/DirectedGraph.h"

#include "Metrics.h"
#include "MetricsAnalysis.h"

#include <set>

using namespace llvm;

namespace {
class MNode;
class MEdge;
using MNodeBase = DGNode<MNode, MEdge>;
using MEdgeBase = DGEdge<MNode, MEdge>;
using MDGBase = DirectedGraph<MNode, MEdge>;

const Value *getMemoryInstrPtr(const Instruction *Inst) {
  if (auto LI = dyn_cast<LoadInst>(Inst)) {
    return LI->getPointerOperand();
  }
  if (auto SI = dyn_cast<StoreInst>(Inst)) {
    return SI->getPointerOperand();
  }
  // todo: add atomics & calls

  return nullptr;
}

bool IsMemInstruction(const Instruction &I) {
  return (dyn_cast_or_null<LoadInst>(&I) != nullptr) ||
         (dyn_cast_or_null<StoreInst>(&I) != nullptr);
}

struct NodeFeatures {
  unsigned MemOps = 0;
  unsigned TotalMemAccessInBytes = 0;
  unsigned ExecutionIntensity = 0;
} DummyNodeFeatures;

class MNode : public MNodeBase {
public:
  enum class NodeKind {
    Root,
    Feature,
    FunctionRoot
  };

  MNode() = delete;
  MNode(const NodeFeatures &F) : MNodeBase(), Features(F) {}
  MNode(const MNode &N) : MNodeBase(N), Features(N.Features), Kind(N.Kind) {}
  MNode(MNode &&N) : MNodeBase(std::move(N)), Features(N.Features), Kind(N.Kind) {}

  //MNode& operator=(const MNode& Other);
  virtual ~MNode() {};

  NodeKind GetKind() const { return Kind; }
  void SetKind(NodeKind K) { Kind = K; }

private:
// move features to derived?
  NodeFeatures Features;
  NodeKind Kind;
};

class RootMNode : public MNode {
public:
  RootMNode() : MNode(DummyNodeFeatures) { SetKind(NodeKind::Root); };
  ~RootMNode() {};

  static bool classof(const MNode *N) {
    return N->GetKind() == NodeKind::Root;
  }
  static bool classof(const RootMNode *N) { return true; }
};

class FeatureMNode : public MNode {
public:
  FeatureMNode(NodeFeatures &F) : MNode(F) { SetKind(NodeKind::Feature); };
  ~FeatureMNode() {};

  static bool classof(const MNode *N) {
    return N->GetKind() == NodeKind::Feature;
  }
  static bool classof(const FeatureMNode *N) { return true; }
};

class FunctionMNode : public MNode {
public:
  FunctionMNode() : MNode(DummyNodeFeatures) { SetKind(NodeKind::FunctionRoot); };
  ~FunctionMNode() {};

  static bool classof(const MNode *N) {
    return N->GetKind() == NodeKind::FunctionRoot;
  }
  static bool classof(const FunctionMNode *N) { return true; }
};

class MEdge : public MEdgeBase {
public:
  MEdge() = delete;
  MEdge(const MEdge &E) : MEdgeBase(E) {}
  MEdge(MEdge &&E) : MEdgeBase(E) {}
  virtual ~MEdge() = 0;
};

class FDGraph {
public:
  FDGraph() {
    auto *N = new RootMNode();
    G.addNode(*N);
    Current = Root = N;
  }
  ~FDGraph() {
    for (auto &N: G) {
      delete N;
    }
  }

  void SetCurrent(MNode *N) { Current = N; }
  MNode *GetCurrent() const { return Current; }
  MNode *GetRoot() const { return Root; }

  void addNode(MNode *N) {
    G.addNode(*N);
  }

  void dump() {
    for (auto &N: G) {
      // todo
    }
  }
private:
  MDGBase G;
  RootMNode *Root;
  MNode *Current;
};

struct Metrics : public ModulePass {
  static char ID;
  Metrics() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M) override;
  virtual bool doInitialization(Module &M) override;
  virtual bool doFinalization(Module &M) override;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  void visitBasicBlock(BasicBlock &B);
  bool runOnFunction(Function &F);
  void visitMemoryInstruction(Instruction &I, NodeFeatures &NF);
  static unsigned SCCCount;

private:
  const DataLayout *DL;
  LLVMContext *Ctx;
  FDGraph FeatureGraph;
};

void Metrics::visitMemoryInstruction(Instruction &I, NodeFeatures &NF) {
  NF.MemOps++;

  auto MemPtr = getMemoryInstrPtr(&I);
  uint64_t AccessSize = 0;

  if (auto LdInst = dyn_cast<LoadInst>(&I)) {
    auto PtrType = LdInst->getPointerOperandType();
    AccessSize = DL->getPointerTypeSize(PtrType);
  }
  if (auto StInst = dyn_cast<StoreInst>(&I)) {
    auto PtrType = StInst->getPointerOperandType();
    AccessSize = DL->getPointerTypeSize(PtrType);
  }

  errs() << "\t\t";
  errs().write_escaped(I.getOpcodeName())
      << " at addr: " << MemPtr << ", size: ";
  errs() << AccessSize << "\n";
}

bool Metrics::runOnFunction(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
  
  // iterate blocks out of any loop
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
    // todo: refactor
    for (LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; ++L) {
      if ((*L)->contains(&(*B)))
        break;
    }
    // TODO: put BB into features graph
    NodeFeatures NF;
    for (Instruction &I : *B) {
      if (IsMemInstruction(I)) {
        visitMemoryInstruction(I, NF);
      }
    }
    auto *N = new FeatureMNode(NF);
    FeatureGraph.addNode(N);
  }

  uint LoopCount = 0;
  for (Loop *LInfo : (*LI)) {
    errs() << "\tLoop " << LoopCount++ << "\n";
    for (auto L = df_begin(LInfo), LE = df_end(LInfo); L != LE; ++L) {
      uint BBNum = 0;

      for (BasicBlock *B : L->blocks()) {
        // todo: visit basic block
        errs() << "\t\tProcessing BB#" << BBNum++ << ", ";
        errs() << "loop depth: " << LI->getLoopDepth(B) << "\n";

        NodeFeatures NF;

        for (Instruction &I : *B) {
          if (IsMemInstruction(I)) {
            visitMemoryInstruction(I, NF);
          }
        }
        auto *N = new FeatureMNode(NF);
        FeatureGraph.addNode(N);
      }
    }
  }

  return false;
}

bool Metrics::doInitialization(Module &M) {
  DL = &M.getDataLayout();
  return false; 
}

bool Metrics::runOnModule(Module &M) {
  //Ctx = (&M)->getContext();
  errs() << "Metrics pass:" << SCCCount << " \n";

  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  
  // incorrect order now
  for (auto SccIter = scc_begin(&CG), SccIterEnd = scc_end(&CG);
       SccIter != SccIterEnd; ++SccIter) {
    errs() << "SCC " << SCCCount++;

    for (CallGraphNode *CGN : *SccIter) {
      if (Function *F = CGN->getFunction()) {
        errs() << "\thas function: ";
        errs().write_escaped((F->getName().str())) << "\n";

        if (!F->isDeclaration()) {
          auto *N = new FunctionMNode();
          FeatureGraph.addNode(N);
          //FeatureGraph.
          runOnFunction(*F);
        }
      }
    }
  }

  return false;
}

void Metrics::visitBasicBlock(BasicBlock &B) {}

bool Metrics::doFinalization(Module &M) { return false; }

} // namespace


char Metrics::ID = 0;
unsigned Metrics::SCCCount = 0;

static RegisterPass<Metrics> X("metrics", "IR metrics collection pass");
