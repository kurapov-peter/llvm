#ifndef LLVM_ANALYSIS_METRICSANALYSIS_H
#define LLVM_ANALYSIS_METRICSANALYSIS_H

namespace llvm {

class ModulePass;

ModulePass *createMetricsAnalysisPass();

} // end namespace llvm

#endif