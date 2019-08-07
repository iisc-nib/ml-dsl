#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include "valuetype.h"
#include "value.h"
#include "valuevisitor.h"
#include "neuronproperty.h"
#include "neuron.h"
#include "layer.h"
#include "network.h"
#include "ir.h"

class CollectMergeableNeuronsIntoEnsemblesVisitor : public NetworkVisitor
{
public:
    CollectMergeableNeuronsIntoEnsemblesVisitor()
    { }
    virtual void Visit(Network& network)
    {
        for (int32_t i=0 ; i<network.GetNumberOfLayers() ; ++i)
        {
            Layer& layer = network.GetLayer(i);
            layer.AcceptVisitor(*this);
        }
    }
    virtual void Visit(Layer& layer)
    {
        Neuron* currentEnsembleRep = nullptr;
        Ensemble *currentEnsemble = nullptr;

        for (int32_t i=0 ; i<layer.GetNumberOfNeurons() ; ++i)
        {
            Neuron& currentNeuron = layer.GetNeuron(i);
            if (currentEnsembleRep == nullptr || !AreNeuronsMergeable(currentNeuron, *currentEnsembleRep))
            {
                currentEnsemble = &(layer.CreateNewEnsemble());
                currentEnsembleRep = &currentNeuron;
            }
            currentEnsemble->AddNeuron(currentNeuron);
        }
    }
    virtual void Visit(Neuron& neuron)
    {
    }
    virtual void Visit(InputNeuron& inputNeuron)
    {
    }
    virtual void Visit(OutputNeuron& outputNeuron)
    {
    }
};


void CollectMergeableNeuronsIntoEnsembles(Network& network)
{
    CollectMergeableNeuronsIntoEnsemblesVisitor collectVisitor;
    network.AcceptVisitor(collectVisitor);
}

static std::string ConstructLayerOutputName(int32_t layerIdx)
{
    std::stringstream strStream;
    strStream << "__layer" << layerIdx << "Result";
    return strStream.str();
}

static VectorType& ConstructLayerOutputType(Layer& layer)
{
    RealType& elemType = *(new RealType());
    VectorType& outputType = *(new VectorType(elemType, layer.GetNumberOfNeurons()));
    return outputType;
}

// Collect all constants into different property bags
class CollectConstantValuesVisitor : public ValueVisitor
{
    std::vector<ConstantValue*> m_constants;

    void VisitBinaryOperation(BinaryOp& binOp)
    {
        binOp.GetLHS().AcceptVisitor(*this);
        binOp.GetRHS().AcceptVisitor(*this);
    }
public:
    std::vector<ConstantValue*>& GetConstants(){ return m_constants;}
    virtual void Visit(Value& value)
    {
        throw std::runtime_error("Visit(Value&) should never be called without a valid override");
    }
    virtual void Visit(IntegerConstant& intConst)
    {
        m_constants.push_back(&intConst);
    }
    virtual void Visit(BooleanConstant& boolConst)
    {
        m_constants.push_back(&boolConst);
    }
    virtual void Visit(RealConstant& realConst)
    {
        m_constants.push_back(&realConst);
    }
    virtual void Visit(RealVectorConstant& realVecConst)
    {
        m_constants.push_back(&realVecConst);
    }
    virtual void Visit(UnaryPlus& unaryPlus)
    {
        unaryPlus.GetOperand().AcceptVisitor(*this);
    }
    virtual void Visit(UnaryMinus& unaryMinus)
    {
        unaryMinus.GetOperand().AcceptVisitor(*this);
    }
    virtual void Visit(BinaryAdd& binaryAdd)
    {
        VisitBinaryOperation(binaryAdd);
    }
    virtual void Visit(BinarySubtract& binarySubtract)
    {
        VisitBinaryOperation(binarySubtract);
    }
    virtual void Visit(BinaryMultiply& binaryMultiply)
    {
        VisitBinaryOperation(binaryMultiply);
    }
    virtual void Visit(BinaryDivide& binaryDivide)
    {
        VisitBinaryOperation(binaryDivide);
    }
    virtual void Visit(GetInputValue& getInput) 
    { }   
    virtual void Visit(Reduction& reduction)
    {
        reduction.GetOperand().AcceptVisitor(*this);
    }
    virtual void Visit(ActivationFunction& function)
    {
        function.GetOperand().AcceptVisitor(*this);
    }

};

class StatementListInsertor
{
    std::list<IRStatement*>& m_stmList;
public:
    StatementListInsertor(std::list<IRStatement*>& stmList)
    :m_stmList(stmList)
    { }
    void Insert(IRStatement& stm) { m_stmList.push_back(&stm); }
};

class ReferenceCreator
{
public:
    virtual Value& operator()(Variable& var) = 0;
};

class VariableRefCreator : public ReferenceCreator
{
public:
    Value& operator()(Variable& var)
    { 
        return var;
    }
};

class IndexVariableRefCreator : public ReferenceCreator
{
    Variable& m_index;
public:
    IndexVariableRefCreator(Variable& index)
        :m_index(index)
    {}
    Value& operator()(Variable& var)
    {
        return IndexedValue::Create(var, m_index);
    }
};

// Collect all constants into different property bags
class ValueIRGenerator : public ValueVisitor
{
    std::map<Value*, Variable*> m_valueToVariableMap;
    std::map<ConstantValue*, ValueSet*> m_constantToValueSetMap;
    Neuron& m_neuron;
    Variable& m_inputVar;
    Variable& m_loopVariable;
    std::list<IRStatement*>& m_stmList;
    int32_t m_varID;

    void AddDefinition(Variable& v)
    {
        VariableDefinition& defn = VariableDefinition::Create(v);
        m_stmList.push_back(&defn);
    }
    void AddVariableForValue(Value& v, Variable& var)
    {
        assert(m_valueToVariableMap.find(&v) == m_valueToVariableMap.end());
        m_valueToVariableMap[&v] = &var;
    }
    std::string GetTempVariableName()
    {
        std::string ret = "_tempVar" + std::to_string(m_varID);
        ++m_varID;
        return ret;
    }
    Variable& CreateTempVariable(ValueType& type)
    {
        auto varName = GetTempVariableName();
        Variable& var = Variable::Create(varName, type);
        AddDefinition(var);
        return var;
    }
    void VisitConstant(ConstantValue& constant)
    {
        if (GetCorrespondingVariable(constant) != nullptr)
            return;
        Variable& var = CreateTempVariable(*(constant.GetType().Clone()));
        AddVariableForValue(constant, var);

        // Create a value set getter
        auto valueSetIter = m_constantToValueSetMap.find(&constant);
        assert (valueSetIter != m_constantToValueSetMap.end());
        GetValue& getVal = GetValue::Create(*(valueSetIter->second), m_loopVariable);
        
        // Add assignment statement
        Assignment& assignmentStm = Assignment::Create(var, getVal);
        m_stmList.push_back(&assignmentStm);
    }
    template<typename T>
    void VisitBinaryOperation(BinaryOp& binOp, T& creationFunc)
    {
        if (GetCorrespondingVariable(binOp) != nullptr)
            return;

        binOp.GetLHS().AcceptVisitor(*this);
        binOp.GetRHS().AcceptVisitor(*this);

        // Create an output variable
        Variable& var = CreateTempVariable(*(binOp.GetType().Clone()));
        AddVariableForValue(binOp, var);

        // Check the type and add a loop if required
        auto codeGenerationParams = GetCodeGenerationParams(binOp);
    
        // Generate statement based on whether or not operands need to be indexed
        Variable& lhs = *(GetCorrespondingVariable(binOp.GetLHS()));
        Variable& rhs = *(GetCorrespondingVariable(binOp.GetRHS()));

        Value& computedValue = creationFunc(codeGenerationParams.refCreator(lhs), codeGenerationParams.refCreator(rhs));
        IRStatement& stm  = Assignment::Create(codeGenerationParams.refCreator(var), computedValue);
        codeGenerationParams.stmListInsertor.Insert(stm);
    }
    struct CodeGenerationParameters
    {
        StatementListInsertor& stmListInsertor;
        ReferenceCreator& refCreator;
    };
    CodeGenerationParameters GetCodeGenerationParams(Value& v)
    {
        VectorType* vecType = dynamic_cast<VectorType*>(&(v.GetType()));
        if (vecType != nullptr)
        {
            ForLoop& forLoop = ForLoop::Create(Constant(0), Constant(vecType->GetLength()));
            m_stmList.push_back(&forLoop);
            auto stmListInsertor = new StatementListInsertor(forLoop.GetStatements());
            auto refCreator = new IndexVariableRefCreator(forLoop.GetIndexVariable());
            CodeGenerationParameters ret = {*stmListInsertor, *refCreator };
            return ret;
        }
        else
        {
            auto stmListInsertor = new StatementListInsertor(m_stmList);
            auto refCreator = new VariableRefCreator;
            CodeGenerationParameters ret = {*stmListInsertor, *refCreator };
            return ret;
        }
    }
public:
    ValueIRGenerator(Neuron& neuron, std::map<ConstantValue*, ValueSet*>& constantToValueSetMap,
                     Variable& loopVar, std::list<IRStatement*>& stmList, Variable& inputVar)
        :m_varID(0), m_constantToValueSetMap(constantToValueSetMap),
         m_loopVariable(loopVar), m_stmList(stmList), m_neuron(neuron), m_inputVar(inputVar)
    {
    }
    Variable* GetCorrespondingVariable(Value& v)
    {
        auto iter = m_valueToVariableMap.find(&v);
        if(iter != m_valueToVariableMap.end())
            return iter->second;
        return nullptr;
    }
    virtual void Visit(Value& value)
    {
        throw std::runtime_error("Visit(Value&) should never be called without a valid override");
    }
    virtual void Visit(IntegerConstant& intConst)
    {
        VisitConstant(intConst);
    }
    virtual void Visit(BooleanConstant& boolConst)
    {
        VisitConstant(boolConst);
    }
    virtual void Visit(RealConstant& realConst)
    {
        VisitConstant(realConst);
    }
    virtual void Visit(RealVectorConstant& realVecConst)
    {
        // TODO should this be further lowered? Is an array assignment ok?
        VisitConstant(realVecConst);
    }
    virtual void Visit(UnaryPlus& unaryPlus)
    {
        if (GetCorrespondingVariable(unaryPlus) != nullptr)
            return;
        Value& operand = unaryPlus.GetOperand();
        operand.AcceptVisitor(*this);
        Variable& operandVar = *(GetCorrespondingVariable(operand));
        AddVariableForValue(unaryPlus, operandVar);
    }
    virtual void Visit(UnaryMinus& unaryMinus)
    {
        if (GetCorrespondingVariable(unaryMinus) != nullptr)
            return;

        unaryMinus.GetOperand().AcceptVisitor(*this);

        // Create an output variable
        Variable& var = CreateTempVariable(*(unaryMinus.GetType().Clone()));
        AddVariableForValue(unaryMinus, var);

        // Check the type and add a loop if required
        auto codeGenerationParams = GetCodeGenerationParams(unaryMinus);
    
        // Generate statement based on whether or not operands need to be indexed
        Variable& input = *(GetCorrespondingVariable(unaryMinus.GetOperand()));
        IRStatement& stm  = Assignment::Create(codeGenerationParams.refCreator(var), UnaryMinus::Create(codeGenerationParams.refCreator(input)));
        codeGenerationParams.stmListInsertor.Insert(stm);
    }
    virtual void Visit(BinaryAdd& binaryAdd)
    {
        VisitBinaryOperation(binaryAdd, BinaryAdd::Create);
    }
    virtual void Visit(BinarySubtract& binarySubtract)
    {
        VisitBinaryOperation(binarySubtract, BinarySubtract::Create);
    }
    virtual void Visit(BinaryMultiply& binaryMultiply)
    {
        VisitBinaryOperation(binaryMultiply, BinaryMultiply::Create);
    }
    virtual void Visit(BinaryDivide& binaryDivide)
    {
        VisitBinaryOperation(binaryDivide, BinaryDivide::Create);
    }
    virtual void Visit(GetInputValue& getInput) 
    {
        if (GetCorrespondingVariable(getInput) != nullptr)
            return;
        Variable& var = CreateTempVariable(*(getInput.GetType().Clone()));
        AddVariableForValue(getInput, var);

        if (VectorType *vectorType = dynamic_cast<VectorType*>(&(getInput.GetType())))
        {
            // FirstIndex = first neuron first input index + Loop index
            // inputVar[0] = prevOutput[FirstIndex]
            // SecondIndex = first neuron second input index + Loop index
            // inputVar[1] = prevOutput[SecondIndex]
            // ...
            for (int32_t i=0 ; i<vectorType->GetLength() ; ++i)
            {
                int32_t neuronInputIndex = m_neuron.GetSources()[i]->GetNeuronID();
                Variable& indexVar = CreateTempVariable(*(new IntegerType));
                Value& indexVal = BinaryAdd::Create(Constant(neuronInputIndex), m_loopVariable);
                IRStatement& indexValAssignment = Assignment::Create(indexVar, indexVal);
                m_stmList.push_back(&indexValAssignment);

                Value& lhs = IndexedValue::Create(var, Constant(i));
                Value& rhs = IndexedValue::Create(m_inputVar, indexVal);
                IRStatement& inputInitialization = Assignment::Create(lhs, rhs);
                m_stmList.push_back(&inputInitialization);
            }
        }
        else
        {
            int32_t neuronInputIndex;
            if (InputNeuron* inputNeuron = dynamic_cast<InputNeuron*>(&m_neuron))
                neuronInputIndex = m_neuron.GetNeuronID();
            else
                neuronInputIndex = m_neuron.GetSources()[0]->GetNeuronID();
            Variable& indexVar = CreateTempVariable(*(new IntegerType));
            Value& indexVal = BinaryAdd::Create(Constant(neuronInputIndex), m_loopVariable);
            IRStatement& indexValAssignment = Assignment::Create(indexVar, indexVal);
            m_stmList.push_back(&indexValAssignment);

            Value& lhs = var;
            Value& rhs = IndexedValue::Create(m_inputVar, indexVal);
            IRStatement& inputInitialization = Assignment::Create(lhs, rhs);
            m_stmList.push_back(&inputInitialization);
        }
    }   
    virtual void Visit(Reduction& reduction)
    {
        if (GetCorrespondingVariable(reduction) != nullptr)
            return;
        
        reduction.GetOperand().AcceptVisitor(*this);
        
        Variable& var = CreateTempVariable(*(reduction.GetType().Clone()));
        AddVariableForValue(reduction, var);

        Variable& inputVar = *(GetCorrespondingVariable(reduction.GetOperand()));
        VectorType* vecType = dynamic_cast<VectorType*>(&(reduction.GetOperand().GetType()));
        assert (vecType != nullptr);

        IRStatement& initStm = Assignment::Create(var, IndexedValue::Create(inputVar, Constant(0)));
        m_stmList.push_back(&initStm);

        ForLoop& forLoop = ForLoop::Create(Constant(1), Constant(vecType->GetLength()));
        Value& iterationValue = BinaryAdd::Create(var, IndexedValue::Create(inputVar, forLoop.GetIndexVariable()));
        IRStatement& assignment = Assignment::Create(var, iterationValue);
        forLoop.AddStatement(assignment);
        m_stmList.push_back(&forLoop);
    }
    virtual void Visit(ActivationFunction& function)
    {
        if (GetCorrespondingVariable(function) != nullptr)
            return;
        
        Value& operand = function.GetOperand();
        operand.AcceptVisitor(*this);
        Variable& operandVar = *(GetCorrespondingVariable(operand));
     
        assert(dynamic_cast<ScalarType*>(&(function.GetType())) != nullptr);
        Variable& var = CreateTempVariable(*(function.GetType().Clone()));
        AddVariableForValue(function, var);
        IRStatement& assignStm = Assignment::Create(var, ActivationFunction::Create(operandVar, function.GetName()));
        m_stmList.push_back(&assignStm);
    }
};

void AddValuesToValueSets(std::vector<ValueSet*>& valueSets, std::vector<ConstantValue*>& constants)
{
    assert(valueSets.size() == constants.size());
    for(size_t i=0; i<constants.size() ; ++i)
    {
        auto valueSet = *valueSets[i];
        valueSet.AddValue(*constants[i]);
    }
}

std::map<ConstantValue*, ValueSet*> CreateValueSetsForEnsemble(Ensemble& ensemble, Function& func, std::vector<ValueSet*>& ensembleValueSets)
{
    auto& firstNeuron = *(ensemble.GetNeurons().front());
    CollectConstantValuesVisitor constantCollector;
    firstNeuron.GetForwardPropagationValue().AcceptVisitor(constantCollector);
    
    std::map<ConstantValue*, ValueSet*> constantToValueSetMap;
    auto& constants = constantCollector.GetConstants();
    for(size_t id=0; id<constants.size() ; ++id)
    {
        ValueSet& valueSet = func.CreateValueSet((int)id, constants[id]->GetType());
        ensembleValueSets.push_back(&valueSet);
        constantToValueSetMap[constants[id]] = &valueSet;
    }
    return constantToValueSetMap;
}

/*
IR code structure
1. Allocate layer 1 output
2. Ensemble1, Layer 1 loop --> layer 1 output
3. Ensemble2, layer 1 loop --> layer 1 output
4. ..
5. Allocate layer 2 ouput
6. Ensemble 1, layer 2 loop --> layer 2 output 
*/
void ConstructIRForEnsemble(Function& func, Ensemble& ensemble, Variable& output, Variable& input)
{
    // 1. Create a ValueSet for all appropriate properties of the neuron (currently assuming its a weighted neuron)
    std::vector<ValueSet*> ensembleValueSets;
    auto constantToValueSetMap = CreateValueSetsForEnsemble(ensemble, func, ensembleValueSets);

    auto& neurons = ensemble.GetNeurons();

    // 0. One loop per ensemble, each loops over all neurons in the ensemble
    auto& firstNeuron = *(neurons.front());
    int32_t baseIndex = firstNeuron.GetLayer().GetNeuronID(firstNeuron);
    ForLoop& ensembleLoop = ForLoop::Create(Constant(baseIndex),
                                            Constant(baseIndex + ensemble.GetNumberOfNeurons()));
    func.AddStatement(ensembleLoop);

    for(size_t i=0; i<neurons.size() ; ++i)
    {
        auto neuron = neurons[i];

        CollectConstantValuesVisitor collectConstants;
        neuron->GetForwardPropagationValue().AcceptVisitor(collectConstants);
        AddValuesToValueSets(ensembleValueSets, collectConstants.GetConstants());
    }

    // 2. Construct IR for the representative neuron for the ensemble
    ValueIRGenerator irGenerator(firstNeuron, constantToValueSetMap, ensembleLoop.GetIndexVariable(), ensembleLoop.GetStatements(), input);
    firstNeuron.GetForwardPropagationValue().AcceptVisitor(irGenerator);

    IndexedValue& indexedValue = IndexedValue::Create(output, ensembleLoop.GetIndexVariable());
    Value& result = *irGenerator.GetCorrespondingVariable(firstNeuron.GetForwardPropagationValue());
    auto& assignmentStm = Assignment::Create(indexedValue, result);
    ensembleLoop.AddStatement(assignmentStm);
}

Function& ConstructIRForNetwork(Network& network)
{
    Layer& inputLayer = network.GetLayer(0);
    VectorType& inputVarType = ConstructLayerOutputType(inputLayer);
    Variable& inputVar = Variable::Create("x", inputVarType);

    Layer& outputLayer = network.GetLayer(network.GetNumberOfLayers()-1);
    VectorType& outputVarType = ConstructLayerOutputType(outputLayer);
    Variable& outputVar = Variable::Create("y", outputVarType);
    
    Function& function = Function::Create(inputVar, outputVar);

    // First create a loop to go over the input layer
    // std::string inputLayerOutputVarName = ConstructLayerOutputName(0);
    // VectorType& inputLayerOutputType = ConstructLayerOutputType(inputLayer);
    // Variable& inputLayerOutput = Variable::Create(inputLayerOutputVarName, inputLayerOutputType);
    
    Variable *prevLayerOutput = &inputVar;

    // Loop over the layers
    // auto layerLoop = ForLoop::Create(Constant(0), Constant(network.GetNumberOfLayers()));
    for (int32_t i=0 ; i<network.GetNumberOfLayers() ; ++i)
    {
        Layer& layer = network.GetLayer(i);
        std::string layerOutputVarName = ConstructLayerOutputName(i); 
        bool lastLayer = i == network.GetNumberOfLayers() - 1;
        Variable& layerOutputVar = lastLayer ? outputVar : Variable::Create(layerOutputVarName, ConstructLayerOutputType(layer));
        
        if(!lastLayer)
        {
            VariableDefinition& outputVarDefnStm = VariableDefinition::Create(layerOutputVar);
            function.AddStatement(outputVarDefnStm);
        }
        
        auto ensembles = layer.GetEnsembles();
        for (int32_t j=0 ; j<ensembles.size() ; ++j)
        {
            auto ensemble = ensembles[j];
            ConstructIRForEnsemble(function, *ensemble, layerOutputVar, *prevLayerOutput);
        }
        prevLayerOutput = &layerOutputVar;
    }

    return function;
}
