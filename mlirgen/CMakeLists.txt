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
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRTransforms)
