// Funciton level analysis pass, to be called by Metrics module pass to retrieve funciton level information.

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct MetricsFunctionLevelAnalysis : public FunctionPass {
        static char ID;
        MetricsFunctionLevelAnalysis() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            errs() << "MetricsFunctionLevelAnalysis: ";
            errs().write_escaped(F.getName()) << "\n";
            return false;
        }
    };
}

char MetricsFunctionLevelAnalysis::ID = 0;

static RegisterPass<MetricsFunctionLevelAnalysis> X("MetricAnalysis", "TBD");

FunctionPass *createMetricsAnalysisPass() {
    return new MetricsFunctionLevelAnalysis();
}