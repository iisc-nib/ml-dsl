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
        m_ostr << " ]";
    }
};

void PrintValueType(ValueType& valueType, std::ostream& ostr)
{
    PrintValueTypeVisitor printVisitor(ostr);
    valueType.AcceptVisitor(printVisitor);
}

