#include <iostream>
#include "valuetype.h"

class PrintValueTypeVisitor : public ValueTypeVisitor
{
    std::ostream& m_ostr;
public:
    PrintValueTypeVisitor(std::ostream& ostr)
        :m_ostr(ostr)
    { }
    virtual void Visit(ScalarType& theType)
    {

    }
	virtual void Visit(BooleanType& theType)
    {
        m_ostr << "[bool]";
    }
	virtual void Visit(IntegerType& theType)
    {
        m_ostr << "[int]";
    }
    virtual void Visit(RealType& theType)
    {
        m_ostr << "[real]";
    }
	virtual void Visit(VectorType& theType)
    {
        m_ostr << "[ Vector ";
        theType.GetElementType().AcceptVisitor(*this);
        int32_t len = theType.GetLength();
        if (len != -1)
            m_ostr << ", " << len;
        m_ostr << " ]";
    }
};

void PrintValueType(ValueType& valueType, std::ostream& ostr)
{
    PrintValueTypeVisitor printVisitor(ostr);
    valueType.AcceptVisitor(printVisitor);
}

class CompareValueTypesVisitor : public ValueTypeVisitor
{
    ValueType* m_type;
    bool m_equal;
public:
    CompareValueTypesVisitor(ValueType& type)
        :m_type(&type), m_equal(true)
    { }
    bool GetResult() { return m_equal; }
    virtual void Visit(ScalarType& theType)
    {
        
    }
	virtual void Visit(BooleanType& theType)
    {
        if (dynamic_cast<BooleanType*>(m_type) == nullptr)
            m_equal = false;
    }
	virtual void Visit(IntegerType& theType)
    {
        if (dynamic_cast<IntegerType*>(m_type) == nullptr)
            m_equal = false;
    }
    virtual void Visit(RealType& theType)
    {
        if (dynamic_cast<RealType*>(m_type) == nullptr)
            m_equal = false;
    }
	virtual void Visit(VectorType& theType)
    {
        VectorType* otherType = dynamic_cast<VectorType*>(m_type);
        if (m_type == nullptr)
            m_equal = false;
        m_type = &otherType->GetElementType();
        theType.GetElementType().AcceptVisitor(*this);
    }
};

bool operator==(ValueType& type1, ValueType& type2)
{
    CompareValueTypesVisitor compareVisitor(type1);
    type2.AcceptVisitor(compareVisitor);
    return compareVisitor.GetResult();
}

