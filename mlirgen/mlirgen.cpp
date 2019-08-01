#include <iostream>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Module.h>
#include <mlir/IR/Function.h>
#include <mlir/IR/StandardTypes.h>
#include <mlir/IR/Builders.h>
#include <mlir/EDSC/Intrinsics.h>
#include <mlir/Support/LLVM.h> // SmallVector
#include <mlir/Target/LLVMIR/ModuleTranslation.h>
using namespace std;

void CreateMainForAddTwoNumbers(mlir::MLIRContext& mlirContext, mlir::OwningModuleRef& module)
{
    llvm::SmallVector<mlir::Type, 0> emptyArgs;
    mlir::IntegerType i32Type = mlir::IntegerType::get(32, &mlirContext);
    auto mainFuncType = mlir::FunctionType::get(emptyArgs, i32Type, &mlirContext);

    // create the main function
    mlir::FuncOp mainFunction = mlir::FuncOp::create(mlir::UnknownLoc::get(&mlirContext), "main", mainFuncType, {});
    mainFunction.addEntryBlock();
    auto builder = llvm::make_unique<mlir::OpBuilder>(mainFunction.getBody());

    // generate constants
    auto valueFive = builder->create<mlir::ConstantOp>(mlir::UnknownLoc::get(&mlirContext), mlir::IntegerAttr::get(i32Type, 5));
    auto valueTen = builder->create<mlir::ConstantOp>(mlir::UnknownLoc::get(&mlirContext), mlir::IntegerAttr::get(i32Type, 10));

    // call 'AddTwoInt32'
    llvm::SmallVector<mlir::Value*, 2> args { valueFive, valueTen };
    // auto result = builder->create<mlir::GenericCallOp>(mlir::UnknownLoc::get(&mlirContext), "AddTwoInt32", args); 

    // add the function to the module
    // module->push_back(mainFunction);
}

void TranslateModuleToLLVM(const mlir::ModuleOp& module)
{
    auto llvmModule = mlir::LLVM::ModuleTranslation::translateModule<>(module);
    // llvmModule->dump();
}

void TestAddTwoNumbers()
{
    mlir::MLIRContext mlirContext;
    mlir::OwningModuleRef module = mlir::ModuleOp::create(mlir::UnknownLoc::get(&mlirContext));    

    // create the function prototype    
    mlir::IntegerType i32Type = mlir::IntegerType::get(32, &mlirContext);
    llvm::SmallVector<mlir::Type, 2> arg_types(2, i32Type);
    auto func_type = mlir::FunctionType::get(arg_types, i32Type, &mlirContext);

    // create the function
    mlir::FuncOp addFunction = mlir::FuncOp::create(mlir::UnknownLoc::get(&mlirContext), "AddTwoInt32", func_type, {}/* attrs*/);;
    addFunction.addEntryBlock();
    auto builder = llvm::make_unique<mlir::OpBuilder>(addFunction.getBody());
	
    // generate the code
    mlir::Value *firstArg = addFunction.getArgument(0);
    mlir::Value *secondArg = addFunction.getArgument(1);

    mlir::Value *result = builder->create<mlir::AddIOp>(mlir::UnknownLoc::get(&mlirContext), firstArg, secondArg);
    builder->create<mlir::ReturnOp>(mlir::UnknownLoc::get(&mlirContext), result);
    
    // add function into the module
    module->push_back(addFunction);

    // create a main function
    CreateMainForAddTwoNumbers(mlirContext, module);

    // dump the MLIR
    cout << "Generated MLIR: " << endl;
    module->dump();

    // convert to LLVM
    TranslateModuleToLLVM(module.get());
}

int main()
{
	// cout <<"Simple program to write a DSL with MLIR" <<endl;
	TestAddTwoNumbers();
	return 0;
}
