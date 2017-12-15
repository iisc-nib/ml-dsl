#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include <iostream>
#include <vector>
#include "valuevisitor.h"

class ValueType;
class ValueVisitor;
class Neuron;

/*
   Value
   	- Constant
		- TODO do we need vector constants?
	- Variable
		- TODO Are variables needed?
	- Arithmetic operators
   		- Unary
		- Binary
	- Relational operators (<, >, <=, >=)
	- Boolean operators (&&, ||, !)
	- (*) Pointwise function (sin, cos etc)
		- TODO How do we call these functions? Do we just use the name and let the loader bind?
	- (*) GetPropertyValue
	- (*) GetInputValue
	- GetVectorLength (?)
	- Conditional expression
	- Dot product (?)
	- (*) Reduction
*/

// TODO Make references and pointers consistent
// TODO Write create methods for each type of Value. So that we can not leak Value objects.
// TODO Write operator overloads so that users can write expressions that look like code (val1 + val2 for example)

class Value
{
protected:
	ValueType *m_type;
public:
    Value()
        :m_type(nullptr)
    { }
	ValueType& GetType() 
    { 
        if (m_type == nullptr)
            InferType();
        return *m_type;
    }
	virtual void InferType() = 0;
	virtual void AcceptVisitor(ValueVisitor& visitor) = 0;
	virtual ~Value() { delete m_type; }
};

class Constant : public Value
{
};

class IntegerConstant : public Constant
{
protected:
	int64_t m_val;
public:
	IntegerConstant(int64_t val)
		:m_val(val)
	{ }
	int64_t GetValue() { return m_val; }
	virtual void InferType() { m_type = new IntegerType; }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static IntegerConstant& Create(int64_t val)
    {
        return *(new IntegerConstant(val));
    }
};

class BooleanConstant : public Constant
{
protected:
	bool m_val;
public:
	BooleanConstant(bool val)
		:m_val(val)
	{ }
	bool GetValue() { return m_val; }
	virtual void InferType() { m_type = new BooleanType; }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static BooleanConstant& Create(bool val)
    {
        return *(new BooleanConstant(val));
    }
};

class RealConstant : public Constant
{
protected:
	double m_val;
public:
	RealConstant(double val)
		:m_val(val)
	{ }
	double GetValue() { return m_val; }
	virtual void InferType() { m_type = new RealType; }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static RealConstant& Create(double val)
    {
        return *(new RealConstant(val));
    }
};

class RealVectorConstant : public Constant
{
protected:
	std::vector<double> m_val;
public:
	RealVectorConstant(std::vector<double>& val)
		:m_val(val)
	{ }
	std::vector<double>& GetValue() { return m_val; }
	virtual void InferType()
    {
        ScalarType& elemType = *(new RealType);
        VectorType* vecType  = new VectorType(elemType);
        vecType->SetLength(m_val.size());
        m_type = vecType;
    }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static RealVectorConstant& Create(std::vector<double>& val)
    {
        return *(new RealVectorConstant(val));
    }
};

class NumericOp : public Value
{
};

class UnaryOp : public NumericOp
{
protected:
	Value *m_operand;
public:
	UnaryOp(Value *operand)
		:m_operand(operand)
	{ }
	Value& GetOperand() { return *m_operand; }
	virtual void InferType() { m_type = m_operand->GetType().Clone(); }
};

class UnaryPlus : public UnaryOp
{
public:
	UnaryPlus(Value *operand)
		:UnaryOp(operand)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
    
    static UnaryPlus& Create(Value& operand)
    {
        return *(new UnaryPlus(&operand));
    }
};

class UnaryMinus : public UnaryOp
{
public:
	UnaryMinus(Value *operand)
		:UnaryOp(operand)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static UnaryMinus& Create(Value& operand)
    {
        return *(new UnaryMinus(&operand));
    }
};

class BinaryOp : public NumericOp
{
protected:
	Value *m_rhs;
	Value *m_lhs;
public:
	BinaryOp(Value *rhs, Value *lhs)
		:m_rhs(rhs), m_lhs(lhs)
	{ }
	Value& GetRHS() { return *m_rhs; }
	Value& GetLHS() { return *m_lhs; }
	virtual void InferType();
};

class BinaryAdd : public BinaryOp
{
public:
	BinaryAdd(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static BinaryAdd& Create(Value& lhs, Value& rhs)
    {
        return *(new BinaryAdd(&lhs, &rhs));
    }
};

class BinarySubtract : public BinaryOp
{
public:
	BinarySubtract(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static BinarySubtract& Create(Value& lhs, Value& rhs)
    {
        return *(new BinarySubtract(&lhs, &rhs));
    }
};

class BinaryMultiply : public BinaryOp
{
public:
	BinaryMultiply(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static BinaryMultiply& Create(Value& lhs, Value& rhs)
    {
        return *(new BinaryMultiply(&lhs, &rhs));
    }
};

class BinaryDivide : public BinaryOp
{
public:
	BinaryDivide(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static BinaryDivide& Create(Value& lhs, Value& rhs)
    {
        return *(new BinaryDivide(&lhs, &rhs));
    }
};

/*
Use constants instead of properties!

class GetProperty : public Value
{
	int32_t m_propertyID;
    Neuron& m_neuron; // [TODO] Should this even be here? This is only for the type inference
public:
	GetProperty(Neuron& neuron, int32_t propertyID)
		:m_neuron(neuron), m_propertyID(propertyID)
	{ }
    virtual void InferType();
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
    Neuron& GetOwningNeuron() { return m_neuron; }
    int32_t GetPropertyID() { return m_propertyID; }

    static GetProperty& Create(Neuron& neuron, int32_t propertyID)
    {
        return *(new GetProperty(neuron, propertyID));
    }
};
*/

class GetInputValue : public Value
{
    Neuron& m_neuron;
public:
    GetInputValue(Neuron& neuron)
        :m_neuron(neuron)
    { }
    virtual void InferType();
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
    Neuron& GetOwningNeuron() { return m_neuron; }

    static GetInputValue& Create(Neuron& neuron)
    {
        return *(new GetInputValue(neuron));
    }
};

class Reduction : public Value
{
public:
    enum ReductionType { Sum, Multiply, Max };
private:
    Value *m_operand;
    ReductionType m_reductionType;
public:
    Reduction(Value *operand, ReductionType reductionType)
        :m_operand(operand)
    { }
    Value& GetOperand() { return *m_operand; }
    ReductionType GetReductionType() { return m_reductionType; }
    virtual void InferType();
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }

    static Reduction& Create(Value& operand, ReductionType reductionType)
    {
        return *(new Reduction(&operand, reductionType));
    }
};

inline BooleanConstant& Constant(bool val)
{
    return BooleanConstant::Create(val);
}

inline IntegerConstant& Constant(int32_t val)
{
    return IntegerConstant::Create(val);
}

inline IntegerConstant& Constant(int64_t val)
{
    return IntegerConstant::Create(val);
}

inline RealConstant& Constant(float val)
{
    return RealConstant::Create(val);
}

inline RealConstant& Constant(double val)
{
    return RealConstant::Create(val);
}

inline RealVectorConstant& Constant(std::vector<double>& val)
{
    return RealVectorConstant::Create(val);
}

inline UnaryPlus& operator+(Value& operand)
{
    return UnaryPlus::Create(operand);
}

inline UnaryMinus& operator-(Value& operand)
{
    return UnaryMinus::Create(operand);
}

inline BinaryAdd& operator+(Value& lhs, Value& rhs)
{
    return BinaryAdd::Create(lhs, rhs);
}

inline BinarySubtract& operator-(Value& lhs, Value& rhs)
{
    return BinarySubtract::Create(lhs, rhs);
}

inline BinaryMultiply& operator*(Value& lhs, Value& rhs)
{
    return BinaryMultiply::Create(lhs, rhs);
}

inline BinaryDivide& operator/(Value& lhs, Value& rhs)
{
    return BinaryDivide::Create(lhs, rhs);
}

void PrintValue(Value& v, std::ostream& ostr);
#endif // _EXPRESSION_H_
