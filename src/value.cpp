#include <map>
#include <string>
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
        int32_t resultLen = std::min(vector1->GetLength(), vector2->GetLength());
        return new VectorType(*elemType, resultLen);
    }
    else if (isType1Vector)
    {
        ScalarType *elemType = dynamic_cast<ScalarType*>(JoinTypes(&(vector1->GetElementType()), type2));
        return new VectorType(*elemType, vector1->GetLength());
    }
    else if (isType2Vector)
    {
        ScalarType *elemType = dynamic_cast<ScalarType*>(JoinTypes(type1, &(vector2->GetElementType())));
        return new VectorType(*elemType, vector2->GetLength());
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

/*
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
*/

void GetInputValue::InferType()
{
    if (dynamic_cast<InputNeuron*>(&m_neuron) != nullptr)
        m_type = new RealType;
    else
    {
        int32_t numInputs = m_neuron.GetNumInputs();
        ScalarType *elemType = new RealType;
        m_type = new VectorType(*elemType, numInputs);
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

class PrintValueVisitor : public ValueVisitor
{
    std::ostream& m_ostr;
    int32_t m_currTemp;
    std::map<Value*, std::string> m_valueToTempMap;

    void SetValueTempName(Value& v, std::string& str)
    {
        m_valueToTempMap[&v] = str;
    }

    std::string GetValueTempName(Value& v)
    {
        std::map<Value*, std::string>::iterator iter = m_valueToTempMap.find(&v);
        if (iter != m_valueToTempMap.end())
            return iter->second;
        else
        {
            v.AcceptVisitor(*this);
            return m_valueToTempMap[&v];
        }
    }

    std::string GetTemp()
    {
        std::string ret = "_t";
        ret += std::to_string(m_currTemp);
        ++m_currTemp;
        return ret;
    }

    void PrintBinaryOp(BinaryOp& binOp, char op)
    {
        const std::string& lhs = GetValueTempName(binOp.GetLHS());
        const std::string& rhs = GetValueTempName(binOp.GetRHS());
        std::string temp = GetTemp();
        m_ostr << temp << " = " << lhs << " " << op << " " << rhs;
        PrintType(binOp);
        m_ostr << std::endl;
        SetValueTempName(binOp, temp);
    }

    void PrintType(Value& v)
    {
        m_ostr << "\t";
        PrintValueType(v.GetType(), m_ostr);
    }
public:
    PrintValueVisitor(std::ostream& ostr)
        :m_ostr(ostr), m_currTemp(0)
    { }
    virtual void Visit(IntegerConstant& intConst)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = " << "int(" << intConst.GetValue() << ")";
        PrintType(intConst);
        m_ostr << std::endl;
        SetValueTempName(intConst, temp);
    }
    virtual void Visit(BooleanConstant& boolConst)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = " << "bool(" << boolConst.GetValue() << ")";
        PrintType(boolConst);
        m_ostr << std::endl;
        SetValueTempName(boolConst, temp);
    }
    virtual void Visit(RealConstant& realConst)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = " << "real(" << realConst.GetValue() << ")";
        PrintType(realConst);
        m_ostr << std::endl;
        SetValueTempName(realConst, temp);
    }
    virtual void Visit(RealVectorConstant& realVecConst)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = " << "realVector( ";
        for (int32_t i=0 ; i<realVecConst.GetValue().size() ; ++i)
            m_ostr << realVecConst.GetValue()[i] << " ";
        m_ostr << ")";
        PrintType(realVecConst);
        m_ostr << std::endl;
        SetValueTempName(realVecConst, temp);
    }
    virtual void Visit(UnaryPlus& unaryPlus)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = " << "+" << GetValueTempName(unaryPlus.GetOperand());
        PrintType(unaryPlus);
        m_ostr << std::endl;
        SetValueTempName(unaryPlus, temp);
    }
    virtual void Visit(UnaryMinus& unaryMinus)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = " << "-" << GetValueTempName(unaryMinus.GetOperand());
        PrintType(unaryMinus);
        m_ostr << std::endl;
        SetValueTempName(unaryMinus, temp);
    }
    virtual void Visit(BinaryAdd& binaryAdd)
    {
        PrintBinaryOp(binaryAdd, '+');
    }
    virtual void Visit(BinarySubtract& binarySubtract)
    {
        PrintBinaryOp(binarySubtract, '-');
    }
    virtual void Visit(BinaryMultiply& binaryMultiply)
    {
        PrintBinaryOp(binaryMultiply, '*');
    }
    virtual void Visit(BinaryDivide& binaryDivide)
    {
        PrintBinaryOp(binaryDivide, '/');
    }
    /*
    virtual void Visit(GetProperty& getProperty)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = GetPropertyValue(" <<  getProperty.GetPropertyID() << ")";
        PrintType(getProperty);
        m_ostr << std::endl;
        SetValueTempName(getProperty, temp);
    }
    */
    virtual void Visit(GetInputValue& getInput)
    {
        std::string temp = GetTemp();
        m_ostr << temp << " = GetInputValue()";
        PrintType(getInput);
        m_ostr << std::endl;
        SetValueTempName(getInput, temp);
    }
    virtual void Visit(Reduction& reduction)
    {
        std::string temp = GetTemp();
        Value& operand = reduction.GetOperand();
        Reduction::ReductionType reductionType = reduction.GetReductionType();
        std::string reductionTypeStr;
        if (reductionType == Reduction::Sum)
            reductionTypeStr = "Sum";
        else if (reductionType == Reduction::Max)
            reductionTypeStr = "Max";
        else if (reductionType == Reduction::Multiply)
            reductionTypeStr = "Mul";
        m_ostr << temp << " = Reduce(" << GetValueTempName(operand) << ", " << reductionTypeStr << ")";
        PrintType(reduction);
        m_ostr << std::endl;
        SetValueTempName(reduction, temp);
    }
};

void PrintValue(Value& v, std::ostream& ostr)
{
    PrintValueVisitor printVisitor(ostr);
    v.AcceptVisitor(printVisitor);
}
