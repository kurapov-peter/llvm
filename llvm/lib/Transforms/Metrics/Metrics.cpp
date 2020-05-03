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
#include "FeatureGraph.h"


using namespace llvm;

namespace {
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
          // top level functions, connect to root
          auto *N = new FunctionMNode(F->getName().str());
          FeatureGraph.addNode(N);
          FeatureGraph.SetCurrent(N);
          auto *E = new ControlEdge(*N);
          FeatureGraph.connect(*(FeatureGraph.GetRoot()), *N, *E);
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
