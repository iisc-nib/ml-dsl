#ifndef _IRSTATEMENTVISITOR_H_
#define _IRSTATEMENTVISITOR_H_

class IRStatement;
class Assignment;
class ForLoop;
class VariableDefinition;

class IRStatementVisitor
{
public:
    virtual void Visit(IRStatement& stm)
    {
        throw std::runtime_error("IRStatementVisitor::Visit(IRStatement&) should not be called without a valid override");
    }
    virtual void Visit(Assignment& assignment) = 0;
    virtual void Visit(ForLoop& forLoop) = 0;
    virtual void Visit(VariableDefinition& varDefinition) = 0;
};

#endif // _IRSTATEMENTVISITOR_H_
