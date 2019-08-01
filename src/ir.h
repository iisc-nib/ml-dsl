#ifndef _IR_H_
#define _IR_H_

// Contains definitions for classes that represent an IR that 
// is a lower form than the "Value" hierarchy of classes. This
// IR contains things like loops and indexing and is the result
// of "lowering" a network object.

#include <list>
#include <string>
#include <vector>
#include "irvaluevisitor.h"
#include "irstatementvisitor.h"
#include "value.h"

class Network;

// Lower level values that can only be used in the lowered IR
class IRValue : public Value
{
public:
    void AcceptVisitor(ValueVisitor& visitor)
    {
        throw std::runtime_error("AcceptVisitor should not be called on IRValues");
    }
    virtual void AcceptIRValueVisitor(IRValueVisitor& visitor) = 0;
};

class Variable : public IRValue
{
    std::string m_name;
public:
    Variable(const std::string& name, ValueType& type)
        :m_name(name)
    {
        m_type = &type;
    }
    void InferType() { /* Nothing to do because type is already specified */ }
    void AcceptIRValueVisitor(IRValueVisitor& visitor) { visitor.Visit(*this); }
    const std::string& GetName() { return m_name; }

    static Variable& Create(const std::string& name, ValueType& type)
    {
        return *(new Variable(name, type));
    }
};

class IndexedValue : public IRValue
{
    Variable& m_var;
    Value& m_indexer;
public:
    IndexedValue(Variable& var, Value& index)
        :m_var(var), m_indexer(index)
    { }
    Variable& GetVariable() { return m_var; }
    Value& GetIndexer() { return m_indexer; }
    void AcceptIRValueVisitor(IRValueVisitor& visitor) { visitor.Visit(*this); }
    virtual void InferType();
    static IndexedValue& Create(Variable& var, Value& index)
    {
        return *(new IndexedValue(var, index));
    }
};

// Represents a group of values with the same type. For example,
// a ValueSet can store the weights of all neurons with the same
// number of inputs.

// The alternative to the approach of using a set of values of the
// same type is to use "property bags" on each neuron. This would
// mean we have arrays of structs that represent all neuron properties.
// The approach we have taken is a struct of arrays. It is not yet 
// clear to me whether one is better than the other.
class ValueSet 
{
    int32_t m_id;
    ValueType& m_elemType;
    std::vector<ConstantValue*> m_values;
public:
    ValueSet(int32_t id, ValueType& elemType)
        :m_id(id), m_elemType(elemType)
    { }
    int32_t GetID() { return m_id; }
    ValueType& GetElementType() { return m_elemType; }
    int32_t AddValue(ConstantValue& constVal);
    ConstantValue& GetValue(int32_t id) { return *m_values[id]; }
};

class GetValue : public IRValue
{
    ValueSet& m_valueSet;
    Value& m_elemID;
public:
    GetValue(ValueSet& valSet, Value& elemID)
        :m_valueSet(valSet), m_elemID(elemID)
    { }
    Value& GetElementID() { return m_elemID; }
    ValueSet& GetValueSet() { return m_valueSet; }
    void AcceptIRValueVisitor(IRValueVisitor& visitor) { visitor.Visit(*this); }
    virtual void InferType()
    {
        m_type = m_valueSet.GetElementType().Clone();
    }
    static GetValue& Create(ValueSet& valSet, Value& elemID)
    {
        return *(new GetValue(valSet, elemID));
    }
};


// TODO Consider moving the IRStatement list functionality that is common to function
// and for loop into a shared class
class Function
{
    Variable& m_inputVar;
    Variable& m_outputVar;
    std::list<ValueSet*> m_valueSets;
    std::list<IRStatement*> m_stmList;
public:
    Function(Variable& inputVar, Variable& outputVar)
        :m_inputVar(inputVar), m_outputVar(outputVar)
    {}
    const std::list<IRStatement*>& GetStatementList() { return m_stmList; }
    void AddStatement(IRStatement& stm) { m_stmList.push_back(&stm); }
    static Function& Create(Variable& inputVar, Variable& outputVar)
    {
        return *(new Function(inputVar, outputVar));
    }
    ValueSet& CreateValueSet(int32_t id, ValueType& elemType)
    {
        auto valueSet = new ValueSet(id, elemType);
        m_valueSets.push_back(valueSet);
        return *valueSet;
    }
};

class IRStatementVisitor;

class IRStatement
{
public:
    virtual void CheckTypes() = 0;
    virtual void AcceptVisitor(IRStatementVisitor& visitor) = 0;
};

class Assignment : public IRStatement
{
    Value& m_lhs; // must be a variable or an indexed value
    Value& m_rhs;
public:
    Assignment(Value& lhs, Value& rhs)
        :m_lhs(lhs), m_rhs(rhs)
    { }
    Value& GetRHS() { return m_rhs; }
    Value& GetLHS() { return m_lhs; }
    void CheckTypes();
    void AcceptVisitor(IRStatementVisitor& visitor) { visitor.Visit(*this); }
    static Assignment& Create(Value& lhs, Value& rhs)
    {
        return *(new Assignment(lhs, rhs));
    }
};

class VariableDefinition : public IRStatement
{

};

class ForLoop : public IRStatement
{
    Value& m_start;
    Value& m_end;
    // Value& m_step; TODO is this needed?
    Variable* m_indexVar;

    std::list<IRStatement*> m_statements;

    static int32_t sLoopIndexNum;
    static std::string GetLoopVarName()
    {
        std::string ret = "__index" + std::to_string(sLoopIndexNum);
        ++sLoopIndexNum;
        return ret;
    }
public:
    ForLoop(Value& start, Value& end)
        :m_start(start), m_end(end)
    {
        m_indexVar = new Variable(GetLoopVarName(), *(new IntegerType));
    }
    ~ForLoop()
    {
        delete m_indexVar;
    }
    void AddStatement(IRStatement& stm) { m_statements.push_back(&stm); }
    void CheckTypes();
    void AcceptVisitor(IRStatementVisitor& visitor) { visitor.Visit(*this); }
    Variable& GetIndexVariable() { return *m_indexVar; }
    Value& GetStart() { return m_start; }
    Value& GetEnd() { return m_end; }
    std::list<IRStatement*>& GetStatements() { return m_statements; }
    static ForLoop& Create(Value& start, Value& end)
    {
        return *(new ForLoop(start, end));
    }
};

void Print(IRStatement& stm, std::ostream& ostr, int32_t indent=0);
Function& ConstructIRForNetwork(Network& network);

#endif // _IR_H_
