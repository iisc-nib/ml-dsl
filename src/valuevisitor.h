#ifndef _VALUEVISITOR_H_
#define _VALUEVISITOR_H_

#include <stdexcept>

// Abstract value classes
class Value;

// Concrete value classes
class IntegerConstant;
class BooleanConstant;
class RealConstant;
class RealVectorConstant;
class UnaryPlus;
class UnaryMinus;
class BinaryAdd;
class BinarySubtract;
class BinaryMultiply;
class BinaryDivide;
// class GetProperty;
class GetInputValue;
class Reduction;

class ValueVisitor
{
public:
    virtual void Visit(Value& value)
    {
        throw std::runtime_error("Visit(Value&) should never be called without a valid override");
    }
    virtual void Visit(IntegerConstant& intConst) = 0;
    virtual void Visit(BooleanConstant& boolConst) = 0;
    virtual void Visit(RealConstant& realConst) = 0;
    virtual void Visit(RealVectorConstant& realVecConst) = 0;
    virtual void Visit(UnaryPlus& unaryPlus) = 0;
    virtual void Visit(UnaryMinus& unaryMinus) = 0;
    virtual void Visit(BinaryAdd& binaryAdd) = 0;
    virtual void Visit(BinarySubtract& binarySubtract) = 0;
    virtual void Visit(BinaryMultiply& binaryMultiply) = 0;
    virtual void Visit(BinaryDivide& binaryDivide) = 0;
    // virtual void Visit(GetProperty& getProperty) = 0;
    virtual void Visit(GetInputValue& getInput) = 0;   
    virtual void Visit(Reduction& reduction) = 0;
};
#endif // _VALUEVISITOR_H_
