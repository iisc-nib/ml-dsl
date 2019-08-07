#include <sstream>
#include "ir.h"

int32_t ForLoop::sLoopIndexNum = 0;

void IndexedValue::InferType()
{
    ValueType& variableType = m_var.GetType();
    VectorType *varVectorType = dynamic_cast<VectorType*>(&variableType);
    if (varVectorType == nullptr)
        throw std::runtime_error("Variable in an indexed value must be a vector");
    ValueType& indexType = m_indexer.GetType();
    ScalarType *indexScalarType = dynamic_cast<ScalarType*>(&indexType);
    if (indexScalarType == nullptr)
        throw std::runtime_error("Index in an indexed value must be a scalar");
    m_type = varVectorType->GetElementType().Clone();
}


int32_t ValueSet::AddValue(ConstantValue& constVal)
{
    if (!(m_elemType == constVal.GetType()))
        throw std::runtime_error("Values in a set must be the same type");
    int32_t id = m_values.size();
    m_values.push_back(&constVal);
    return id;
}

void Assignment::CheckTypes()
{
    if (!(m_rhs.GetType() == m_lhs.GetType()))
        throw std::runtime_error("Assignment : LHS and RHS types must be identical");
    ValueType& lhsType = m_lhs.GetType();
    if (dynamic_cast<ScalarType*>(&lhsType) == nullptr)
        throw std::runtime_error("Assignment : LHS must be a scalar value");
    // check that lhs is an l-value
    bool lhsIsLValue = (dynamic_cast<Variable*>(&m_lhs) != nullptr) ||
                       (dynamic_cast<IndexedValue*>(&m_lhs) != nullptr);
    if (!lhsIsLValue)
        throw std::runtime_error("Assignment : LHS must be a variable or indexed reference");
}

void ForLoop::CheckTypes()
{
    ValueType& startType = m_start.GetType();
    if (dynamic_cast<ScalarType*>(&startType) == nullptr)
        throw std::runtime_error("ForLoop : Start must be a scalar value");
    ValueType& endType = m_end.GetType();
    if (dynamic_cast<ScalarType*>(&endType) == nullptr)
        throw std::runtime_error("ForLoop : End must be a scalar value");
    for (std::list<IRStatement*>::iterator stm=m_statements.begin() ; stm!=m_statements.end() ; ++stm)
    {
        (*stm)->CheckTypes();
    }
}

class PrintIRStatementVisitor : public IRStatementVisitor
{
    std::ostream& m_ostr;
    int32_t m_indent;

    void Indent()
    {
        for(int32_t i=0 ; i<m_indent ; ++i)
            m_ostr << "\t";
    }
public:
    PrintIRStatementVisitor(std::ostream& ostr, int32_t indent=0)
        :m_ostr(ostr), m_indent(indent)
    { }
    virtual void Visit(Assignment& assignment)
    {
        // std::string rhsTempName = PrintValue(assignment.GetRHS(), m_ostr, m_indent);
        // Indent();
        // PrintValueExpression(assignment.GetLHS(), m_ostr);
        // m_ostr << " = " << rhsTempName << std::endl;
        PrintValue(assignment.GetRHS(), m_ostr, m_indent, assignment.GetLHS());
    }
    virtual void Visit(ForLoop& forLoop)
    {
        Indent();
        m_ostr << "for " << forLoop.GetIndexVariable().GetName() << " = ";
        PrintValueExpression(forLoop.GetStart(), m_ostr);
        m_ostr << " : ";
        PrintValueExpression(forLoop.GetEnd(), m_ostr);
        m_ostr << "\n";
        Indent();
        m_ostr << "{\n";
        m_indent += 1;
        std::list<IRStatement*>& stms = forLoop.GetStatements();
        for (std::list<IRStatement*>::iterator iter = stms.begin(); iter!=stms.end() ; ++iter)
            (*iter)->AcceptVisitor(*this);
        m_indent -= 1;
        Indent();
        m_ostr << "}\n";
    }
    virtual void Visit(VariableDefinition& varDefinition)
    {
        Indent();
        m_ostr << "Define :" << varDefinition.GetVariable().GetName() << "\t";
        PrintValueType(varDefinition.GetVariable().GetType(), m_ostr);
        m_ostr << std::endl;
    }
};

void Print(IRStatement& stm, std::ostream& ostr, int32_t indent)
{
    PrintIRStatementVisitor printVisitor(ostr, indent);
    stm.AcceptVisitor(printVisitor);
}
