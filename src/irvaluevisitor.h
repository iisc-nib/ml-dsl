#ifndef _IRVALUEVISITOR_H_
#define _IRVALUEVISITOR_H_

#include "valuevisitor.h"

class Variable;
class IndexedValue;
class GetValue;

class IRValueVisitor : public ValueVisitor
{
public:
    virtual void Visit(Variable& variable) = 0;
    virtual void Visit(IndexedValue& indexedVal) = 0;
    virtual void Visit(GetValue& getValue) = 0;
};

#endif // _IRVALUEVISITOR_H_
