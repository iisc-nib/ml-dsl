#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

class ValueType;
class ValueVisitor;

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
	- Pointwise function (sin, cos etc)
	- GetProperty(Value, length)
	- GetInputValue
	- GetInputLength
	- Conditional expression
	- Dot product
	- Reduction (?)
*/

class Value
{
protected:
	ValueType *m_type;
public:
	ValueType& GetType() { return *m_type; }
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
};

class RealConstant : public Constant
{
protected:
	double m_val;
public:
	RealConstant(bool val)
		:m_val(val)
	{ }
	double GetValue() { return m_val; }
	virtual void InferType() { m_type = new RealType; }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
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
	Value* GetOperand() { return m_operand; }
	virtual void InferType() { m_type = m_operand->GetType().Clone(); }
};

class UnaryPlus : public UnaryOp
{
public:
	UnaryPlus(Value *operand)
		:UnaryOp(operand)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
};

class UnaryMinus : public UnaryOp
{
public:
	UnaryMinus(Value *operand)
		:UnaryOp(operand)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
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
	Value* GetRHS() { return m_rhs; }
	Value* GetLHS() { return m_lhs; }
	virtual void InferTypes();
};

class BinaryAdd : public BinaryOp
{
public:
	BinaryAdd(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
};

class BinarySubtract : public BinaryOp
{
public:
	BinarySubtract(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
};

class BinaryMultiply : public BinaryOp
{
public:
	BinaryMultiply(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
};

class BinaryDivide : public BinaryOp
{
public:
	BinaryDivide(Value *rhs, Value *lhs)
		:BinaryOp(rhs, lhs)
	{ }
	virtual void AcceptVisitor(ValueVisitor& visitor) { visitor.Visit(*this); }
};

#endif // _EXPRESSION_H_
