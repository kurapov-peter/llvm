#include "llvm/InitializePasses.h"

#define DEBUG_TYPE "metrics"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/LoopInfo.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SCCIterator.h"

#include "llvm/ADT/Statistic.h"

#include "Metrics.h"
#include "MetricsAnalysis.h"


using namespace llvm;

namespace {
struct Metrics : public ModulePass {
  struct Info {
    unsigned MemOps = 0;
  };
  static char ID;
  Metrics() : ModulePass(ID) {
    //initializeMetricsPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnModule(Module &M) override;
  virtual bool doInitialization(Module &M) override;
  virtual bool doFinalization(Module &M) override;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    //AU.addRequired<CallGraphSCCPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  void visitBasicBlock(BasicBlock &B);
  bool runOnFunction(Function &F);
  static unsigned SCCCount;

private:
  const DataLayout *DL;
};
} // namespace

char Metrics::ID = 0;
unsigned Metrics::SCCCount = 0;

/*INITIALIZE_PASS_BEGIN(Metrics, "metrics", "Metrics", true, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(Metrics, "metrics", "Metrics", true, true)*/

static RegisterPass<Metrics> X("metrics", "IR metrics collection pass");

static const Value *getMemoryInstrPtr(const Instruction *Inst) {
  if (auto LI = dyn_cast<LoadInst>(Inst)) {
    return LI->getPointerOperand();
  }
  if (auto SI = dyn_cast<StoreInst>(Inst)) {
    return SI->getPointerOperand();
  }
  // todo: add atomics

  return nullptr;
}

bool Metrics::doInitialization(Module &M) {
  DL = &M.getDataLayout();
  return false; 
}

bool Metrics::runOnModule(Module &M) {
  errs() << "Metrics pass:" << SCCCount << " \n";

  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();

  for (auto SccIter = scc_begin(&CG), SccIterEnd = scc_end(&CG);
       SccIter != SccIterEnd; ++SccIter) {
    errs() << "SCC " << SCCCount++;

    for (CallGraphNode *CGN : *SccIter) {
      if (Function *F = CGN->getFunction()) {
        errs() << "\thas function: ";
        errs().write_escaped((F->getName().str())) << "\n";

        if (!F->isDeclaration()) {
          runOnFunction(*F);
        }
      }
    }
  }

  return false;
}

bool Metrics::runOnFunction(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();

  for (Loop *LInfo : (*LI)) {
    for (auto L = df_begin(LInfo), LE = df_end(LInfo); L != LE; ++L) {
      uint BBNum = 0;

      for (BasicBlock *B : L->blocks()) {
        errs() << "\t\tProcessing BB#" << BBNum++ << ", ";
        errs() << "loop depth: " << LI->getLoopDepth(B) << "\n";

        for (Instruction &I : *B) {
          if (auto LdInst = dyn_cast<LoadInst>(&I)) {
            auto MemPointer = getMemoryInstrPtr(&I);

            errs() << "\t\t";
            errs().write_escaped(I.getOpcodeName())
                << " at addr: " << MemPointer << ", size: ";

            auto PointerType = LdInst->getPointerOperandType();
            uint64_t LoadSize = DL->getPointerTypeSize(PointerType);

            errs() << LoadSize << "\n";
          }
        }
      }
    }
  }

  return false;
}

void Metrics::visitBasicBlock(BasicBlock &B) {}

bool Metrics::doFinalization(Module &M) { return false; }
