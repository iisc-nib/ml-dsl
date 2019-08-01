add_custom_target(MLIRGen)
set_target_properties(MLIRGen PROPERTIES FOLDER Examples)

macro(add_mlirgen_chapter name)
  add_dependencies(MLIRGen ${name})
  add_llvm_example(${name} ${ARGN})
endmacro(add_mlirgen_chapter name)

set(LLVM_LINK_COMPONENTS
  Support
)

add_mlirgen_chapter(mlirgen
  mlirgen.cpp  
)

  include_directories(include/)
  target_link_libraries(mlirgen
  PRIVATE
  MLIRAffineOps 
  MLIRAnalysis 
  MLIRControlFlowToCFG 
  MLIRDialect 
  MLIREDSC 
  MLIRExecutionEngine 
  MLIRFxpMathOps 
  MLIRGPU 
  MLIRGPUtoNVVMTransforms 
  MLIRIR 
  MLIRJitRunner 
  MLIRLLVMIR 
  MLIRLinalg 
  MLIRLoopOps 
  MLIRLoopsToGPU 
  MLIRMlirOptLib 
  MLIRNVVMIR 
  MLIROptMain 
  MLIRParser 
  MLIRPass 
  MLIRQuantOps 
  MLIRQuantizerFxpMathConfig 
  MLIRQuantizerSupport 
  MLIRQuantizerTransforms 
  MLIRSDBM 
  MLIRSPIRV 
  MLIRSPIRVConversion 
  MLIRSPIRVSerialization 
  MLIRStandardOps 
  MLIRStandardToLLVM 
  MLIRSupport 
  MLIRTargetLLVMIR 
  MLIRTargetLLVMIRModuleTranslation 
  MLIRTargetNVVMIR 
  MLIRTestDialect 
  MLIRTestTransforms 
  MLIRTransformUtils 
  MLIRTransforms 
  MLIRTranslateClParser 
  MLIRTranslation 
  MLIRTypeUtilities 
  MLIRVectorOps
)
