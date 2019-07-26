#include <iostream>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Module.h>
#include <mlir/IR/Function.h>
#include <mlir/IR/StandardTypes.h>
#include <mlir/IR/Builders.h>
#include <mlir/EDSC/Intrinsics.h>

using namespace std;

void TestAddTwoNumbers()
{
	mlir::MLIRContext mlirContext;
	mlir::OwningModuleRef module = mlir::ModuleOp::create(mlir::UnknownLoc::get(&mlirContext));    

    // create the function prototype    
    mlir::IntegerType i32Type = mlir::IntegerType::get(32, &mlirContext);
    llvm::SmallVector<mlir::Type, 2> arg_types(2, i32Type);
    auto func_type = mlir::FunctionType::get(arg_types, i32Type, &mlirContext);

    // create the function
	mlir::FuncOp addFunction = mlir::FuncOp::create(mlir::UnknownLoc::get(&mlirContext), "Add", func_type, {}/* attrs*/);;
    addFunction.addEntryBlock();
    auto builder = llvm::make_unique<mlir::OpBuilder>(addFunction.getBody());
	
    // generate the code
    mlir::Value *firstArg = addFunction.getArgument(0);
    mlir::Value *secondArg = addFunction.getArgument(1);

    mlir::Value *result = builder->create<mlir::AddIOp>(mlir::UnknownLoc::get(&mlirContext), firstArg, secondArg);
    builder->create<mlir::ReturnOp>(mlir::UnknownLoc::get(&mlirContext), result);
    
    // add function into the module
    module->push_back(addFunction);

    module->dump();
}

int main()
{
	// cout <<"Simple program to write a DSL with MLIR" <<endl;
	TestAddTwoNumbers();
	return 0;
}
