#include <iostream>
#include <list>
#include <functional>
#include "Node.h"
#include "Object.h"

using std::cout;
using std::endl;

struct ReturnException 
{ 
    std::any result; 
};
struct BreakException {};
struct ContinueException {};

static std::map<std::string, std::any> global;
static std::list<std::list<std::map<std::string, std::any>>> local;
static std::map<std::string, std::shared_ptr<Function>> functionTable;
extern std::map<std::string, std::function<std::any(std::vector<std::any>)>> builtinFunctionTable;

static auto Indent(int32 _depth)
{
    cout << std::string(static_cast<size_t>(_depth * 2), ' ');
}

void PrintSyntaxTree(Program* program)
{
    for (auto& node : program->m_vecFunction)
        node->PrintInfo(0);
}

void Interpret(std::shared_ptr<Program> _pProgram)
{
    functionTable.clear();
    global.clear();
    local.clear();
    for (auto& node : _pProgram->m_vecFunction) {
        functionTable[node->m_strName] = node;
    }
    if (functionTable["main"] == nullptr) {
        return;
    }
    local.emplace_back().emplace_front();
    try {
        functionTable["main"]->Interpret();
    }
    catch (ReturnException) {}
    catch (BreakException) {}
    catch (ContinueException) {}
    local.pop_back();
}

void Function::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "FUNCTION " << m_strName << ": " << endl;
    if (m_vecParameter.size()) {
        Indent(_depth + 1); cout << "PARAMETERS:";
        for (auto& name : m_vecParameter) {
            cout << name << " ";
        }
        cout << endl;
    }
    Indent(_depth + 1); cout << "BLOCK:" << endl;
    for (auto& node : m_vecBlock) {
        node->PrintInfo(_depth + 2);
    }
}

void Function::Interpret()
{
    for (auto& pNode : m_vecBlock) {
        pNode->Interpret();
    }
}

void For::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "FOR:" << endl;
    Indent(_depth + 1); cout << "VARIABLE:" << endl;
    m_pVariable->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "CONDITION:" << endl;
    m_pCondition->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "EXPRESSION:" << endl;
    m_pExpression->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "BLOCK:" << endl;
    for (auto& node : m_vecBlock) {
        node->PrintInfo(_depth + 2);
    }
}

void For::Interpret()
{
    local.back().emplace_front();
    m_pVariable->Interpret();
    while (true) {
        auto result = m_pCondition->Interpret();
        if (Object::IsTrue(result) == false) {
            break;
        }
        try {
            for (auto& pNode : m_vecBlock) {
                pNode->Interpret();
            }
        }
        catch (ContinueException) {
            /* empty */
        }
        catch (BreakException) {
            break;
        }
        m_pExpression->Interpret();
    }
    local.back().pop_front();
}

void If::PrintInfo(int32 _depth) 
{
    for (size_t i = 0; i < conditions.size(); i++) {
        Indent(_depth); cout << (i == 0 ? "IF:" : "ELIF:") << endl;
      
        Indent(_depth + 1); cout << "CONDITION:" << endl;
        conditions[i]->PrintInfo(_depth + 2);
        
        Indent(_depth + 1); cout << "BLOCK:" << endl;
        for (auto& node : blocks[i])
            node->PrintInfo(_depth + 2);
    }
    if (m_vecElseBlock.size() == 0) {
        return;
    }

    Indent(_depth); cout << "ELSE:" << endl;
    for (auto& node : m_vecElseBlock) {
        node->PrintInfo(_depth + 1);
    }
}

void If::Interpret()
{
}

void Variable::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "VAR " << m_strName << ":" << endl;
    if (m_pExpression) {
        // 없을 수도 있지
        m_pExpression->PrintInfo(_depth + 1);
    }
}

void Variable::Interpret()
{
    local.back().front()[m_strName] = m_pExpression->Interpret();
}

void Print::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << (lineFeed ? "PRINT_LINE" : "PRINT:") << endl;
    for (auto& node : m_vecArgument) {
        node->PrintInfo(_depth + 1);
    }
}

void Print::Interpret()
{
    for (auto& pNode : m_vecArgument) {
        auto anyValue = pNode->Interpret();
        std::cout << anyValue;
    }
    if (lineFeed) cout << endl;
}

void Return::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "RETURN:" << endl;
    m_pExpression->PrintInfo(_depth + 1);
}

void Return::Interpret()
{
    throw ReturnException{ m_pExpression->Interpret() };
}

void Break::PrintInfo(int32 _depth) 
{
    Indent(_depth); cout << "BREAK" << endl;
}

void Break::Interpret()
{
}

void Continue::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "CONTINUE" << endl;
}

void Continue::Interpret()
{
}

void ExpressionStatement::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "EXPRESSION:" << endl;
    m_pExpression->PrintInfo(_depth + 1);
}

void ExpressionStatement::Interpret()
{
    m_pExpression->Interpret();
}

void Or::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "OR:" << endl;
    
    Indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

std::any Or::Interpret()
{
    return Object::IsTrue(m_pLhs->Interpret()) ? true : m_pRhs->Interpret();;
}

void And::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "AND:" << endl;

    Indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);

    Indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

std::any And::Interpret()
{
    return Object::IsFalse(m_pLhs->Interpret()) ? false : m_pRhs->Interpret();
}

void Relational::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << ToString(m_eKind) << ":" << endl;

    Indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);

    Indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

std::any Relational::Interpret()
{
    auto lValue = m_pLhs->Interpret();
    auto rValue = m_pRhs->Interpret();

    auto IsFunc = [&lValue, &rValue](std::function<bool(std::any)> _isFunc) {
        if (_isFunc(lValue) && _isFunc(rValue)) {
            return true;
        }
        return false;
    };

    if (m_eKind == EKind::Equal && IsFunc(Object::IsNull)) {
        return true;
    }
    if (m_eKind == EKind::Equal && IsFunc(Object::IsBoolean)) {
        return Object::ToBoolean(lValue) == Object::ToBoolean(rValue);
    }
    if (m_eKind == EKind::Equal && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) == Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::Equal && IsFunc(Object::IsString)) {
        return Object::ToString(lValue) == Object::ToString(rValue);
    }
    if (m_eKind == EKind::NotEqual && IsFunc(Object::IsNull)) {
        return false;
    }
    if (m_eKind == EKind::NotEqual && Object::IsNull(lValue) || Object::IsNull(rValue)) {
        return true;
    }
    if (m_eKind == EKind::NotEqual && IsFunc(Object::IsBoolean)) {
        return Object::ToBoolean(lValue) != Object::ToBoolean(rValue);
    }
    if (m_eKind == EKind::NotEqual && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) != Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::NotEqual && IsFunc(Object::IsString)) {
        return Object::ToString(lValue) != Object::ToString(rValue);
    }
    if (m_eKind == EKind::LessThan && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) < Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::GreaterThan && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) > Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::LessOrEqual && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) <= Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::GreaterOrEqual && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) >= Object::ToNumber(rValue);
    }
    return false;
}

void Arithmetic::PrintInfo(int32 _depth) 
{
    Indent(_depth); cout << ToString(m_eKind) << ":" << endl;
    
    Indent(_depth + 1); cout << "LHS:" << endl;
    m_pLhs->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "RHS:" << endl;
    m_pRhs->PrintInfo(_depth + 2);
}

std::any Arithmetic::Interpret()
{
    auto lValue = m_pLhs->Interpret();
    auto rValue = m_pRhs->Interpret();
    
    auto IsFunc = [&lValue, &rValue](std::function<bool(std::any)> _isFunc) {
        if (_isFunc(lValue) && _isFunc(rValue)) {
            return true;
        }
        return false;
        };

    if (m_eKind == EKind::Add && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) + Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::Add && IsFunc(Object::IsString)) {
        return Object::ToString(lValue) + Object::ToString(rValue);
    }   
    if (m_eKind == EKind::Subtract && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) - Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::Multiply && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(lValue) * Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::Divide && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(rValue) == 0 ? 0.0 : Object::ToNumber(lValue) / Object::ToNumber(rValue);
    }
    if (m_eKind == EKind::Modulo && IsFunc(Object::IsNumber)) {
        return Object::ToNumber(rValue) == 0 ? Object::ToNumber(lValue) : 
            fmod(Object::ToNumber(lValue), Object::ToNumber(rValue));
    }
    return 0.0;
}

void Unary::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << ToString(m_eKind) << endl;
    m_pSub->PrintInfo(_depth + 1);
}

std::any Unary::Interpret()
{
    auto value = m_pSub->Interpret();
    if (m_eKind == EKind::Add && Object::IsNumber(value)) {
        return abs(Object::ToNumber(value));
    }
    if (m_eKind == EKind::Subtract && Object::IsNumber(value)) {
        return Object::ToNumber(value) * -1;
    }
    return 0.0;
}

void GetElement::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "GET_ELEMENT:" << endl;
    
    Indent(_depth + 1); cout << "SUB:" << endl;
    m_pSub->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "INDEX:" << endl;
    m_pIndex->PrintInfo(_depth + 2);
}

std::any GetElement::Interpret()
{
    auto object = m_pSub->Interpret();
    auto index = m_pIndex->Interpret();
    if (Object::IsArray(object) && Object::IsNumber(index)) {
        return Object::GetValueOfArray(object, index);
    }
    if (Object::IsMap(object) && Object::IsString(index)) {
        return Object::GetValueOfMap(object, index);
    }
    return nullptr;
}

void SetElement::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "SET_ELEMENT:" << endl;
    
    Indent(_depth + 1); cout << "SUB:" << endl;
    m_pSub->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "INDEX:" << endl;
    m_pIndex->PrintInfo(_depth + 2);
    
    Indent(_depth + 1); cout << "VALUE:" << endl;
    m_pValue->PrintInfo(_depth + 2);
}

std::any SetElement::Interpret()
{
    auto object = m_pSub->Interpret();
    auto index = m_pIndex->Interpret();
    auto value = m_pValue->Interpret();
    if (Object::IsArray(object) && Object::IsNumber(index)) {
        return Object::SetValueOfArray(object, index, value);
    }
    if (Object::IsMap(object) && Object::IsString(index)) {
        return Object::SetValueOfMap(object, index, value);
    }
    return nullptr;
}

void Call::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "CALL:" << endl;
    
    Indent(_depth + 1); cout << "EXPRESSION:" << endl;
    m_pSub->PrintInfo(_depth + 2);
    
    for (auto& node : m_vecArgument) {
        Indent(_depth + 1); cout << "ARGUMENT:" << endl;
        node->PrintInfo(_depth + 2);
    }
}

std::any Call::Interpret()
{
    auto value = m_pSub->Interpret();
    if (Object::IsBuiltinFunction(value)) {
        std::vector<std::any> values;
        for (size_t i = 0; i < m_vecArgument.size(); i++) {
            values.push_back(m_vecArgument[i]->Interpret());
        }
        return Object::ToBuiltinFunction(value)(values);
    }
    if (Object::IsFunction(value) == false) {
        return nullptr;
    }
    std::map<std::string, std::any> mapParameter;
    for (size_t i = 0; i < m_vecArgument.size(); i++) {
        auto name = Object::ToFunction(value)->m_vecParameter[i];
        mapParameter[name] = m_vecArgument[i]->Interpret();
    }
    local.emplace_back().push_front(mapParameter);
    try {
        Object::ToFunction(value)->Interpret();
    }
    catch (ReturnException exception) {
        local.pop_back();
        return exception.result;
    }
    local.pop_back();
    return nullptr;
}

void NullLiteral::PrintInfo(int32 _depth) 
{
    Indent(_depth); cout << "null" << endl;
}

std::any NullLiteral::Interpret()
{
    return nullptr;
}

void BooleanLiteral::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << (m_bValue ? "true" : "false") << endl;
}

std::any BooleanLiteral::Interpret()
{
    return m_bValue;
}

void NumberLiteral::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << m_dValue << endl;
}

std::any NumberLiteral::Interpret()
{
    return m_dValue;
}

void StringLiteral::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "\"" << m_strValue << "\"" << endl;
}

std::any StringLiteral::Interpret()
{
    return m_strValue;
}

void ArrayLiteral::PrintInfo(int32 _depth) 
{
    Indent(_depth); cout << "[" << endl;
    for (auto& node : m_vecValue) {
        node->PrintInfo(_depth + 1);
    }
    Indent(_depth); cout << "]" << endl;
}

std::any ArrayLiteral::Interpret()
{
    auto result = std::make_shared<Array>();
    for (auto& pNode : m_vecValue) {
        result->m_vecValue.push_back(pNode->Interpret());
    }
    return result;
}

void MapLiteral::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "{" << endl;
    for (auto& [key, value] : m_mapValue) {
        cout << key << ": ";
        value->PrintInfo(_depth + 1);
    }
    Indent(_depth); cout << "}" << endl;
}

std::any MapLiteral::Interpret()
{
    auto result = std::make_shared<Map>();
    for (auto& [key, value] : m_mapValue) {
        result->m_mapValue.insert_or_assign(key, value->Interpret());
    }
    return result;
}

void GetVariable::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "GET_VARIABLE: " << m_strName << endl;
}

std::any GetVariable::Interpret()
{
    for (auto& vecVariable : local.back()) {
        if (vecVariable.count(m_strName)) {
            return vecVariable[m_strName];
        }
    }
    if (global.count(m_strName)) {
        return global[m_strName];
    }
    if (functionTable.count(m_strName)) {
        return functionTable[m_strName];
    }
    if (builtinFunctionTable.count(m_strName)) {
        return builtinFunctionTable[m_strName];
    }
    return nullptr;
}

void SetVariable::PrintInfo(int32 _depth)
{
    Indent(_depth); cout << "SET_VARIABLE: " << m_strName << endl;
    m_pValue->PrintInfo(_depth + 1);
}

std::any SetVariable::Interpret()
{
    for (auto& variables : local.back()) {
        if (variables.count(m_strName)) {
            return variables[m_strName] = m_pValue->Interpret();
        }
    }
    return global[m_strName] = m_pValue->Interpret();
}
