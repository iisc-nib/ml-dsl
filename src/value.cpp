#include <map>
#include <string>
#include <sstream>
#include "valuetype.h"
#include "value.h"
#include "neuron.h"
#include "neuronproperty.h"
#include "ir.h"

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

void ActivationFunction::InferType()
{
    ValueType& operandType = m_operand->GetType();
    ScalarType* operandScalarType = dynamic_cast<ScalarType*>(&operandType);
    if (operandScalarType == nullptr)
    {
        throw std::runtime_error("Activation function argument must be a scalar type");
    }
    m_type = operandScalarType->Clone();
}

class PrintValueVisitor : public IRValueVisitor
{
    friend std::string PrintValue(Value& value, std::ostream& ostr, int32_t indent);

    std::ostream& m_ostr;
    int32_t m_currTemp;
    int32_t m_indent;
    std::map<Value*, std::string> m_valueToTempMap;
    Value* m_topLevelValuePtr;
    Value* m_topLevelLHSValue;

    void SetValueTempName(Value& v, const std::string& str)
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
            v.AcceptIRValueVisitor(*this);
            return m_valueToTempMap[&v];
        }
    }

    std::string GetTemp(Value& v)
    {
        if (&v == m_topLevelValuePtr)
        {
            // [TODO] This is a horrible hack to be able to print a custom LHS for an assignment statement.
            // I will remove it at some point in the future.
            std::stringstream strStream;
            PrintValueExpression(*m_topLevelLHSValue, strStream);
            std::string lhsStr = strStream.str();
            return lhsStr;
        }
        std::string ret = "_t";
        ret += std::to_string(m_currTemp);
        ++m_currTemp;
        return ret;
    }

    void PrintBinaryOp(BinaryOp& binOp, char op)
    {
        const std::string& lhs = GetValueTempName(binOp.GetLHS());
        const std::string& rhs = GetValueTempName(binOp.GetRHS());
        std::string temp = GetTemp(binOp);
        Indent();
        m_ostr << temp << " = " << lhs << " " << op << " " << rhs;
        PrintType(binOp);
        m_ostr << std::endl;
        SetValueTempName(binOp, temp);
    }

    void PrintType(Value& v)
    {
        m_ostr << "\t\t\t";
        PrintValueType(v.GetType(), m_ostr);
    }

    void Indent()
    {
        for (int32_t i=0; i<m_indent ; ++i)
            m_ostr << "\t";
    }
public:
    PrintValueVisitor(std::ostream& ostr, int32_t indent = 0)
        :m_ostr(ostr), m_currTemp(0), m_indent(indent), m_topLevelValuePtr(nullptr), m_topLevelLHSValue(nullptr)
    { }
    PrintValueVisitor(std::ostream& ostr, Value& topLevelVal, Value& topLevelLHSValue, int32_t indent = 0)
        :m_ostr(ostr), m_currTemp(0), m_indent(indent), m_topLevelValuePtr(&topLevelVal), m_topLevelLHSValue(&topLevelLHSValue)
    { }
    virtual void Visit(IntegerConstant& intConst)
    {
        Indent();
        std::string temp = GetTemp(intConst);
        m_ostr << temp << " = " << "int(" << intConst.GetValue() << ")";
        PrintType(intConst);
        m_ostr << std::endl;
        SetValueTempName(intConst, temp);
    }
    virtual void Visit(BooleanConstant& boolConst)
    {
        Indent();
        std::string temp = GetTemp(boolConst);
        m_ostr << temp << " = " << "bool(" << boolConst.GetValue() << ")";
        PrintType(boolConst);
        m_ostr << std::endl;
        SetValueTempName(boolConst, temp);
    }
    virtual void Visit(RealConstant& realConst)
    {
        Indent();
        std::string temp = GetTemp(realConst);
        m_ostr << temp << " = " << "real(" << realConst.GetValue() << ")";
        PrintType(realConst);
        m_ostr << std::endl;
        SetValueTempName(realConst, temp);
    }
    virtual void Visit(RealVectorConstant& realVecConst)
    {
        Indent();
        std::string temp = GetTemp(realVecConst);
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
        std::string operand = GetValueTempName(unaryPlus.GetOperand());

        Indent();
        std::string temp = GetTemp(unaryPlus);
        m_ostr << temp << " = " << "+" << operand;
        PrintType(unaryPlus);
        m_ostr << std::endl;
        SetValueTempName(unaryPlus, temp);
    }
    virtual void Visit(UnaryMinus& unaryMinus)
    {
        std::string operand = GetValueTempName(unaryMinus.GetOperand());
        Indent();
        std::string temp = GetTemp(unaryMinus);
        m_ostr << temp << " = " << "-" << operand;
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
        Indent();
        std::string temp = GetTemp(getInput);
        m_ostr << temp << " = GetInputValue()";
        PrintType(getInput);
        m_ostr << std::endl;
        SetValueTempName(getInput, temp);
    }
    virtual void Visit(Reduction& reduction)
    {
        std::string temp = GetTemp(reduction);
        Value& operand = reduction.GetOperand();
        Reduction::ReductionType reductionType = reduction.GetReductionType();
        std::string reductionTypeStr;
        if (reductionType == Reduction::Sum)
            reductionTypeStr = "Sum";
        else if (reductionType == Reduction::Max)
            reductionTypeStr = "Max";
        else if (reductionType == Reduction::Multiply)
            reductionTypeStr = "Mul";
        std::string operandTemp = GetValueTempName(operand);
        Indent();
        m_ostr << temp << " = Reduce(" << operandTemp << ", " << reductionTypeStr << ")";
        PrintType(reduction);
        m_ostr << std::endl;
        SetValueTempName(reduction, temp);
    }
    virtual void Visit(ActivationFunction& function)
    {
        std::string operand = GetValueTempName(function.GetOperand());
        Indent();
        std::string temp = GetTemp(function);
        m_ostr << temp << " = " << function.GetName() << "(" << operand << ")";
        PrintType(function);
        m_ostr << std::endl;
        SetValueTempName(function, temp);
    }
    virtual void Visit(Variable& variable)
    {
        // This hack is needed when we have to print an assignment that is just a variable
        // TODO there is still a bug in that we don't print anything when the value passed to 
        // this visitor is jusst a variable and no topLevel is specified.
        if (&variable == m_topLevelValuePtr)
        {
            Indent();
            std::string temp = GetTemp(variable);
            m_ostr << temp << " = " << variable.GetName();
            PrintType(variable);
            m_ostr << std::endl;
        }        
        SetValueTempName(variable, variable.GetName());
    }
    virtual void Visit(IndexedValue& indexedVal)
    {
        std::string temp = GetTemp(indexedVal);
        std::string indexerTemp = GetValueTempName(indexedVal.GetIndexer());
        Indent();
        m_ostr << temp << " = " << indexedVal.GetVariable().GetName() << "[" << indexerTemp << "]";
        PrintType(indexedVal);
        m_ostr << std::endl;
        SetValueTempName(indexedVal, temp);
    }
    virtual void Visit(GetValue& getValue)
    {
        std::string valueID = GetValueTempName(getValue.GetElementID());
        Indent();
        std::string temp = GetTemp(getValue);
        m_ostr << temp << " = " << "GetValue("<< getValue.GetValueSet().GetID() << ", " << valueID << ")";
        PrintType(getValue);
        m_ostr << std::endl;
        SetValueTempName(getValue, temp);
    }
};

std::string PrintValue(Value& v, std::ostream& ostr, int32_t indent)
{
    PrintValueVisitor printVisitor(ostr, indent);
    v.AcceptIRValueVisitor(printVisitor);
    return printVisitor.GetValueTempName(v);
}

void PrintValue(Value& v, std::ostream& ostr, int32_t indent, Value& topLevelLHS)
{
    PrintValueVisitor printVisitor(ostr, v, topLevelLHS, indent);
    v.AcceptIRValueVisitor(printVisitor);
}
class PrintValueExpressionVisitor : public IRValueVisitor
{
    std::ostream& m_ostr;

    void PrintBinaryOp(BinaryOp& binOp, char op)
    {
        m_ostr << "(";
        binOp.GetLHS().AcceptIRValueVisitor(*this);
        m_ostr << op; 
        binOp.GetRHS().AcceptIRValueVisitor(*this);
        m_ostr << ")";
    }

public:
    PrintValueExpressionVisitor(std::ostream& ostr)
        :m_ostr(ostr)
    { }
    virtual void Visit(IntegerConstant& intConst)
    {
        m_ostr << "int(" << intConst.GetValue() << ")";
    }
    virtual void Visit(BooleanConstant& boolConst)
    {
        m_ostr << "bool(" << boolConst.GetValue() << ")";
    }
    virtual void Visit(RealConstant& realConst)
    {
        m_ostr << "real(" << realConst.GetValue() << ")";
    }
    virtual void Visit(RealVectorConstant& realVecConst)
    {
        m_ostr << "realVector( ";
        for (int32_t i=0 ; i<realVecConst.GetValue().size() ; ++i)
            m_ostr << realVecConst.GetValue()[i] << " ";
        m_ostr << ")";
    }
    virtual void Visit(UnaryPlus& unaryPlus)
    {
        m_ostr << "(+";
        unaryPlus.GetOperand().AcceptIRValueVisitor(*this);
        m_ostr << ")";
    }
    virtual void Visit(UnaryMinus& unaryMinus)
    {
        m_ostr << "(-";
        unaryMinus.GetOperand().AcceptIRValueVisitor(*this);
        m_ostr << ")";
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
        m_ostr << "GetInputValue()";
    }
    virtual void Visit(Reduction& reduction)
    {
        Value& operand = reduction.GetOperand();
        Reduction::ReductionType reductionType = reduction.GetReductionType();
        std::string reductionTypeStr;
        if (reductionType == Reduction::Sum)
            reductionTypeStr = "Sum";
        else if (reductionType == Reduction::Max)
            reductionTypeStr = "Max";
        else if (reductionType == Reduction::Multiply)
            reductionTypeStr = "Mul";
        m_ostr << "Reduce(";
        operand.AcceptIRValueVisitor(*this);
        m_ostr << ", " << reductionTypeStr << ")";
    }
    virtual void Visit(ActivationFunction& function)
    {
        m_ostr << function.GetName() << "(";
        function.GetOperand().AcceptIRValueVisitor(*this);
        m_ostr << ")";
    }
    virtual void Visit(Variable& variable)
    {
        m_ostr << variable.GetName();
    }
    virtual void Visit(IndexedValue& indexedVal)
    {
        m_ostr << indexedVal.GetVariable().GetName() << "[";
        indexedVal.GetIndexer().AcceptIRValueVisitor(*this);
        m_ostr << "]";
    }
    virtual void Visit(GetValue& getValue)
    {
        m_ostr << "GetValue("<< getValue.GetValueSet().GetID() << ", ";
        getValue.GetElementID().AcceptIRValueVisitor(*this);
        m_ostr << ")"; 
    }
};

void PrintValueExpression(Value& v, std::ostream& ostr)
{
    PrintValueExpressionVisitor printVisitor(ostr);
    v.AcceptIRValueVisitor(printVisitor);
}


class ValueStructuralCompareVisitor : public ValueVisitor
{
    class StoreAndResetValuePtr
    {
        Value *m_storedVal;
        Value **m_ptrToReset;
    public:
        StoreAndResetValuePtr(Value** ptr)
            :m_storedVal(*ptr), m_ptrToReset(ptr)
        { }
        ~StoreAndResetValuePtr()
        {
            *m_ptrToReset = m_storedVal;
        }
    };
    Value *m_val;
    bool m_equal;

    template<typename T>
    void HandleBinaryOperator(T& val)
    {
        StoreAndResetValuePtr resetVal(&m_val);
        T *otherValue = dynamic_cast<T*>(m_val);
        if (otherValue == nullptr || !(otherValue->GetType() == val.GetType()))
        {
            m_equal = false;
            return;
        }
        m_val = &(otherValue->GetRHS());
        val.GetRHS().AcceptVisitor(*this);
        if (m_equal == false)
            return;
        m_val = &(otherValue->GetLHS());
        val.GetLHS().AcceptVisitor(*this);
    }
public:
    ValueStructuralCompareVisitor(Value& v)
        : m_val(&v), m_equal(true)
    { }
    bool GetResult() { return m_equal; }
    virtual void Visit(IntegerConstant& intConst)
    {
        if (dynamic_cast<IntegerConstant*>(m_val) == nullptr)
            m_equal = false;
    }
    virtual void Visit(BooleanConstant& boolConst)
    {
        if (dynamic_cast<BooleanConstant*>(m_val) == nullptr)
            m_equal = false;
    }
    virtual void Visit(RealConstant& realConst)
    {
        if (dynamic_cast<RealConstant*>(m_val) == nullptr)
            m_equal = false;
    }
    virtual void Visit(RealVectorConstant& realVecConst)
    {
        RealVectorConstant* vectorVal = dynamic_cast<RealVectorConstant*>(m_val);
        if (vectorVal == nullptr || vectorVal->GetValue().size() != realVecConst.GetValue().size())
            m_equal = false;
    }
    virtual void Visit(UnaryPlus& unaryPlus)
    {
        StoreAndResetValuePtr resetVal(&m_val);
        UnaryPlus *unaryPlusVal = dynamic_cast<UnaryPlus*>(m_val);
        if (unaryPlusVal == nullptr)
        {
            m_equal = false;
            return;
        }
        m_val = &(unaryPlusVal->GetOperand());
        unaryPlus.GetOperand().AcceptVisitor(*this);
    }
    virtual void Visit(UnaryMinus& unaryMinus)
    {
        StoreAndResetValuePtr resetVal(&m_val);
        UnaryMinus *unaryMinusVal = dynamic_cast<UnaryMinus*>(m_val);
        if (unaryMinusVal == nullptr)
        {
            m_equal = false;
            return;
        }
        m_val = &(unaryMinusVal->GetOperand());
        unaryMinus.GetOperand().AcceptVisitor(*this);
    }
    virtual void Visit(BinaryAdd& binaryAdd)
    {
        HandleBinaryOperator(binaryAdd);
    }
    virtual void Visit(BinarySubtract& binarySubtract)
    {
        HandleBinaryOperator(binarySubtract);
    }
    virtual void Visit(BinaryMultiply& binaryMultiply)
    {
        HandleBinaryOperator(binaryMultiply);
    }
    virtual void Visit(BinaryDivide& binaryDivide)
    {
        HandleBinaryOperator(binaryDivide);
    }
    virtual void Visit(GetInputValue& getInput)
    {
        if (!(getInput.GetType() == m_val->GetType()))
            m_equal = false;
    }
    virtual void Visit(Reduction& reduction)
    {
        Reduction *otherReduction = dynamic_cast<Reduction*>(m_val);
        if (otherReduction == nullptr || !(otherReduction->GetType() == reduction.GetType()) || 
            otherReduction->GetReductionType()!=reduction.GetReductionType())
        {
            m_equal = false;
            return;
        }
    }
    virtual void Visit(ActivationFunction& function)
    {
        ActivationFunction *otherFunction = dynamic_cast<ActivationFunction*>(m_val);
        if (otherFunction == nullptr || !(otherFunction->GetType() == function.GetType()) ||
            otherFunction->GetName() != function.GetName())
        {
            m_equal = false;
            return;
        }
    }
};


bool AreValuesStructurallyIdentical(Value& v1, Value& v2)
{
    ValueStructuralCompareVisitor compareVisitor(v1);
    v2.AcceptVisitor(compareVisitor);
    return compareVisitor.GetResult();
}
