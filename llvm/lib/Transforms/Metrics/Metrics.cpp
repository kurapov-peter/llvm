#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/LoopInfo.h"

#include "Metrics.h"
#include "MetricsAnalysis.h"


using namespace llvm;
#define DEBUG_TYPE "metrics"

namespace {
struct Metrics : public CallGraphSCCPass {
  struct Info {
    unsigned MemOps = 0;
  };
  static char ID;
  Metrics() : CallGraphSCCPass(ID) {}

  virtual bool runOnSCC(CallGraphSCC &SCC) override;
  virtual bool doInitialization(CallGraph &CG) override;
  virtual bool doFinalization(CallGraph &CG) override;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
  }

  static unsigned SCCCount;
};
} // namespace

char Metrics::ID = 0;
unsigned Metrics::SCCCount = 0;

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

bool Metrics::doInitialization(CallGraph &CG) { return false; }

bool Metrics::runOnSCC(CallGraphSCC &CGSCC) {
  SCCCount++;
  // Shouldn't be used like that. Do not create new instance!
  DataLayout *DL = new DataLayout(&(CGSCC.getCallGraph().getModule()));
  //errs().write_escaped(DL->getStringRepresentation()) <<"'\n";

  errs() << "Metrics:" << SCCCount << " \n";
  for (CallGraphNode *CGN : CGSCC) {
    if (Function *F = CGN->getFunction()) {
      errs() << "\thas function is: ";
      errs().write_escaped((F->getName().str())) << "\n";

      LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

      for (BasicBlock &B : (*F)) {
        for (Instruction &I : B) {
          if (auto LdInst = dyn_cast<LoadInst>(&I)) {
              auto MemPointer = getMemoryInstrPtr(&I);
              errs() << "\t\t";
              errs().write_escaped(I.getOpcodeName()) << " at addr: " << MemPointer << ", size: ";
              auto PointerType = LdInst->getPointerOperandType();
              uint64_t LoadSize = DL->getPointerTypeSize(PointerType);
              errs() << LoadSize << "\n";
          }
        }

      }
    }
  }


  delete DL;

  return false;
}


bool Metrics::doFinalization(CallGraph &CG) {
  return false;
}

