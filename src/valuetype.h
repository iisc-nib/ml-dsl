#ifndef _NEURONPROPERTYTYPE_H_
#define _NEURONPROPERTYTYPE_H_

class ValueType;
class ScalarType;
class BooleanType;
class IntegerType;
class RealType;
class VectorType;

class ValueTypeVisitor
{
public:
	virtual void Visit(ScalarType& theType) = 0;
	virtual void Visit(BooleanType& theType) = 0;
	virtual void Visit(IntegerType& theType) = 0;
	virtual void Visit(RealType& theType) = 0;
	virtual void Visit(VectorType& theType) = 0;
};

class ValueType
{
public:
	virtual void AcceptVisitor(ValueTypeVisitor& visitor) = 0;
	virtual ValueType* Clone() = 0;
    virtual ~ValueType() { }
protected:
	ValueType() { }
};

class ScalarType : public ValueType
{
public:
	virtual void AcceptVisitor(ValueTypeVisitor& visitor) { visitor.Visit(*this); }
protected:
	ScalarType() { }
};

class BooleanType : public ScalarType
{
public:
	BooleanType()
	{ }
	virtual void AcceptVisitor(ValueTypeVisitor& visitor) { visitor.Visit(*this); }
	virtual ValueType* Clone() { return new BooleanType(*this); }
};

class IntegerType : public ScalarType
{
public:
	IntegerType() { }
	virtual void AcceptVisitor(ValueTypeVisitor& visitor) { visitor.Visit(*this); }
	virtual ValueType* Clone() { return new IntegerType(*this); }
};

class RealType : public ScalarType
{
public:
	RealType() { }
	virtual void AcceptVisitor(ValueTypeVisitor& visitor) { visitor.Visit(*this); }
	virtual ValueType* Clone() { return new RealType(*this); }
};

class VectorType : public ValueType
{
	ScalarType& m_elemType;
    int32_t m_len;
public:
	VectorType(ScalarType& elem)
		:m_elemType(elem), m_len(-1)
	{ }
	VectorType(ScalarType& elem, int32_t len)
		:m_elemType(elem), m_len(len)
	{ }
	ScalarType& GetElementType() { return m_elemType; }
	virtual void AcceptVisitor(ValueTypeVisitor& visitor) { visitor.Visit(*this); }
	virtual ValueType* Clone(){ return new VectorType(*dynamic_cast<ScalarType*>(m_elemType.Clone()), m_len); }

    int32_t GetLength() { return m_len; }
    void SetLength(int32_t len) { m_len = len; }
    ~VectorType() { delete &m_elemType; }
};

void PrintValueType(ValueType& type, std::ostream& ostr);
bool operator==(ValueType& type1, ValueType& type2);

#endif // _NEURONPROPERTYTYPE_H_
