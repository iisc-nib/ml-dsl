#include "valuetype.h"
#include "value.h"
#include "neuron.h"
#include "neuronproperty.h"

ValueType* JoinScalarTypes(ValueType* type1, ValueType* type2)
{
    BooleanType* boolType1 = dynamic_cast<BooleanType*>(type1);
    IntegerType* intType1 = dynamic_cast<IntegerType*>(type1);
    RealType* realType1 = dynamic_cast<RealType*>(type1);
    
    BooleanType* boolType2 = dynamic_cast<BooleanType*>(type2);
    IntegerType* intType2 = dynamic_cast<IntegerType*>(type2);
    RealType* realType2 = dynamic_cast<RealType*>(type2);

    if (boolType1 && boolType2)
    {
        return new BooleanType;
    }
    else if (realType1)
    {
        if (realType2 || intType2)
            return new RealType;
        else
            throw std::runtime_error("Invalid binary operator argument types");
    }
    else if (realType2)
    {
        if (intType1)
            return new RealType;
        else
            throw std::runtime_error("Invalid binary operator argument types");
    }
    else if (intType1 && intType2)
    {
        return new IntegerType;
    }
    else
    {
        throw std::runtime_error("Invalid binary operator argument types");
    }
}

ValueType* JoinTypes(ValueType* type1, ValueType* type2)
{
    VectorType *vector1 = dynamic_cast<VectorType*>(type1);
    bool isType1Vector = vector1 != nullptr;
    
    VectorType *vector2 = dynamic_cast<VectorType*>(type2);
    bool isType2Vector = vector2 != nullptr;
    
    if (isType1Vector && isType2Vector)
    {
        ScalarType *elemType = dynamic_cast<ScalarType*>(JoinTypes(&(vector1->GetElementType()), &(vector2->GetElementType())));
        return new VectorType(*elemType);
    }
    else if (isType1Vector)
    {
        ScalarType *elemType = dynamic_cast<ScalarType*>(JoinTypes(&(vector1->GetElementType()), type2));
        return new VectorType(*elemType);
    }
    else if (isType2Vector)
    {
        ScalarType *elemType = dynamic_cast<ScalarType*>(JoinTypes(type1, &(vector2->GetElementType())));
        return new VectorType(*elemType);
    }
    else
    {
        return JoinScalarTypes(type1, type2);
    }
}

void BinaryOp::InferType()
{
    ValueType& rhsType = m_rhs->GetType();
    ValueType& lhsType = m_lhs->GetType();
    
    m_type = JoinTypes(&rhsType, &lhsType);
}

void GetProperty::InferType()
{
    NeuronProperty& neuronProperty = m_neuron.GetNeuronProperty(m_propertyID);
    if (dynamic_cast<ScalarDoubleNeuronProperty*>(&neuronProperty) != nullptr)
        m_type = new RealType();
    else if (dynamic_cast<VectorDoubleNeuronProperty*>(&neuronProperty) != nullptr)
    {
        ScalarType *elemType = new RealType();
        m_type = new VectorType(*elemType);
    }
    else
        throw std::runtime_error("Unknown property type");
}

void GetInputValue::InferType()
{
    if (dynamic_cast<InputNeuron*>(&m_neuron) != nullptr)
        m_type = new RealType;
    else
    {
        ScalarType *elemType = new RealType;
        m_type = new VectorType(*elemType);
    }
}

void Reduction::InferType()
{
    ValueType& operandType = m_operand->GetType();
    VectorType* operandVectorType = dynamic_cast<VectorType*>(&operandType);
    if (operandVectorType == nullptr)
    {
        throw std::runtime_error("Reduction argument must be a vector type");
    }
    m_type = operandVectorType->GetElementType().Clone();
}

