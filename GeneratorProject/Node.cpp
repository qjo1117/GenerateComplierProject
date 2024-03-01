#include <iostream>
#include "Node.h"

using std::cout;
using std::endl;

static auto indent(int32 _depth)
{
    cout << std::string(static_cast<size_t>(_depth * 2), ' ');
}

auto printSyntaxTree(Program* program) -> void {
    for (auto& node : program->m_vecFunction)
        node->PrintInfo(0);
}

auto Function::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "FUNCTION " << m_strName << ": " << endl;
    if (m_vecParameter.size()) {
        indent(_depth + 1); cout << "PARAMETERS:";
        for (auto& name : m_vecParameter) {
            cout << name << " ";
        }
        cout << endl;
    }
    indent(_depth + 1); cout << "BLOCK:" << endl;
    for (auto& node : m_vecBlock) {
        node->PrintInfo(_depth + 2);
    }
}

auto For::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "FOR:" << endl;
    indent(_depth + 1); cout << "VARIABLE:" << endl;
    m_pVariable->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "CONDITION:" << endl;
    m_pCondition->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "EXPRESSION:" << endl;
    m_pExpression->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "BLOCK:" << endl;
    for (auto& node : m_vecBlock) {
        node->PrintInfo(_depth + 2);
    }
}

auto If::PrintInfo(int32 _depth)->void {
    for (size_t i = 0; i < conditions.size(); i++) {
        indent(_depth); cout << (i == 0 ? "IF:" : "ELIF:") << endl;
      
        indent(_depth + 1); cout << "CONDITION:" << endl;
        conditions[i]->PrintInfo(_depth + 2);
        
        indent(_depth + 1); cout << "BLOCK:" << endl;
        for (auto& node : blocks[i])
            node->PrintInfo(_depth + 2);
    }
    if (m_vecElseBlock.size() == 0) {
        return;
    }

    indent(_depth); cout << "ELSE:" << endl;
    for (auto& node : m_vecElseBlock) {
        node->PrintInfo(_depth + 1);
    }
}

auto Variable::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "VAR " << m_strName << ":" << endl;
    m_pExpression->PrintInfo(_depth + 1);
}

auto Print::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << (lineFeed ? "PRINT_LINE" : "PRINT:") << endl;
    for (auto& node : m_vecArgument) {
        node->PrintInfo(_depth + 1);
    }
}

auto Return::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "RETURN:" << endl;
    m_pExpression->PrintInfo(_depth + 1);
}

auto Break::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "BREAK" << endl;
}

auto Continue::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "CONTINUE" << endl;
}

auto ExpressionStatement::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "EXPRESSION:" << endl;
    m_pExpression->PrintInfo(_depth + 1);
}

auto Or::PrintInfo(int32 _depth)->void {
    indent(_depth); cout << "OR:" << endl;
    
    indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

void And::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "AND:" << endl;

    indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);

    indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

void Relational::PrintInfo(int32 _depth)
{
    indent(_depth); cout << ToString(m_eKind) << ":" << endl;

    indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);

    indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

void Arithmetic::PrintInfo(int32 _depth) 
{
    indent(_depth); cout << ToString(m_eKind) << ":" << endl;
    
    indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

void Unary::PrintInfo(int32 _depth)
{
    indent(_depth); cout << ToString(m_eKind) << endl;
    m_pSub->PrintInfo(_depth + 1);
}

void GetElement::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "GET_ELEMENT:" << endl;
    
    indent(_depth + 1); cout << "SUB:" << endl;
    m_pSub->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "INDEX:" << endl;
    m_pIndex->PrintInfo(_depth + 2);
}

void SetElement::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "SET_ELEMENT:" << endl;
    
    indent(_depth + 1); cout << "SUB:" << endl;
    m_pSub->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "INDEX:" << endl;
    m_pIndex->PrintInfo(_depth + 2);
    
    indent(_depth + 1); cout << "VALUE:" << endl;
    m_pValue->PrintInfo(_depth + 2);
}

void Call::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "CALL:" << endl;
    
    indent(_depth + 1); cout << "EXPRESSION:" << endl;
    m_pSub->PrintInfo(_depth + 2);
    
    for (auto& node : m_vecArgument) {
        indent(_depth + 1); cout << "ARGUMENT:" << endl;
        node->PrintInfo(_depth + 2);
    }
}

void NullLiteral::PrintInfo(int32 _depth) 
{
    indent(_depth); cout << "null" << endl;
}

void BooleanLiteral::PrintInfo(int32 _depth)
{
    indent(_depth); cout << (m_bValue ? "true" : "false") << endl;
}

void NumberLiteral::PrintInfo(int32 _depth)
{
    indent(_depth); cout << m_dValue << endl;
}

void StringLiteral::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "\"" << m_strValue << "\"" << endl;
}

void ArrayLiteral::PrintInfo(int32 _depth) 
{
    indent(_depth); cout << "[" << endl;
    for (auto& node : m_vecValue) {
        node->PrintInfo(_depth + 1);
    }
    indent(_depth); cout << "]" << endl;
}

void MapLiteral::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "{" << endl;
    for (auto& [key, value] : m_vecValue) {
        cout << key << ": ";
        value->PrintInfo(_depth + 1);
    }
    indent(_depth); cout << "}" << endl;
}

void GetVariable::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "GET_VARIABLE: " << m_strName << endl;
}

void SetVariable::PrintInfo(int32 _depth)
{
    indent(_depth); cout << "SET_VARIABLE: " << m_strName << endl;
    m_pValue->PrintInfo(_depth + 1);
}

