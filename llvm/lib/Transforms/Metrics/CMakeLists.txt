add_llvm_library( LLVMMetrics MODULE
  Metrics.cpp
  MetricsAnalysis.cpp
  
  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )