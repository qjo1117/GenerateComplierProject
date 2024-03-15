#include <iostream>
#include <list>
#include <functional>
#include <regex>
#include "Node.h"
#include "Object.h"

#include "Application.h"

using std::endl;

struct ReturnException 
{ 
    std::any result; 
};
struct BreakException {};
struct ContinueException {};

static std::string Indent(int32 _depth)
{
    return std::string(static_cast<size_t>(_depth * 2), ' ');
}

void PrintSyntaxTree(Program* program)
{
    for (auto& pNode : program->m_vecFunction) {
        pNode->PrintInfo(0);
    }
}

void Interpreter::Interpret(std::shared_ptr<Program> _pProgram)
{
    // Clear
    GET_INTERPRETER().m_mapFunctionTable.clear();
    GET_INTERPRETER().m_mapGlobal.clear();
    GET_INTERPRETER().m_listLocalFrame.clear();
    for (auto& node : _pProgram->m_vecFunction) {
        GET_INTERPRETER().m_mapFunctionTable[node->m_strName] = node;
    }
    if (GET_INTERPRETER().m_mapFunctionTable["main"] == nullptr) {
        std::cout << "Cannot find main function\n";
        return;
    }
    GET_INTERPRETER().m_listLocalFrame.emplace_back().emplace_front();
    try {
        GET_INTERPRETER().m_mapFunctionTable["main"]->Interpret();
    }
    catch (ReturnException) {}
    catch (BreakException) {}
    catch (ContinueException) {}
    GET_INTERPRETER().m_listLocalFrame.pop_back();
}

Interpreter::Interpreter()
{
    m_mapBuiltinFunctionTable = {
        {"length", [](vector<any> values)->any {
          if (values.size() == 1 && Object::IsArray(values[0])) {
              return static_cast<float64>(Object::ToArray(values[0])->m_vecValue.size());
          }
          if (values.size() == 1 && Object::IsMap(values[0])) {
              return static_cast<float64>(Object::ToMap(values[0])->m_mapValue.size());
          }
          return 0.0;
        }},
        {"push", [](vector<any> values)->any {
          if (values.size() == 2 && Object::IsArray(values[0])) {
            Object::ToArray(values[0])->m_vecValue.push_back(values[1]);
            return values[0];
          }
          return nullptr;
        }},
        {"pop", [](vector<any> values)->any {
          if (values.size() == 1 && Object::IsArray(values[0]) && Object::ToArray(values[0])->m_vecValue.size() != 0) {
            auto result = Object::ToArray(values[0])->m_vecValue.back();
            Object::ToArray(values[0])->m_vecValue.pop_back();
            return result;
          }
          return nullptr;
        }},
        {"erase", [](vector<any> values)->any {
          if (values.size() == 2 && Object::IsMap(values[0]) && Object::IsString(values[1]) &&
              Object::ToMap(values[0])->m_mapValue.count(Object::ToString(values[1]))) {
            auto result = Object::ToMap(values[0])->m_mapValue.at(Object::ToString(values[1]));
            Object::ToMap(values[0])->m_mapValue.erase(Object::ToString(values[1]));
            return result;
          }
          return nullptr;
        }},
        {"clock", [](vector<any> values)->any {
          return static_cast<float64>(clock());
        }},
        {"sqrt", [](vector<any> values)->any {
          return sqrt(Object::ToNumber(values[0]));
        }},
    };
}

std::string Function::PrintInfo(int32 _depth)
{
    std::string strResult 
        = Indent(_depth) + "FUNCTION " + m_strName + " : \n";
    if (m_vecParameter.size()) {
        strResult += Indent(_depth + 1); 
        strResult += "PARAMETERS:";
        for (auto& name : m_vecParameter) {
            strResult += name + " ";
        }
        strResult += "\n";
    }
    strResult += Indent(_depth + 1) + "BLOCK: \n";
    for (auto& node : m_vecBlock) {
        strResult += node->PrintInfo(_depth + 2);
    }
    return strResult;
}

void Function::Interpret()
{
    for (auto& pNode : m_vecBlock) {
        pNode->Interpret();
    }
}

std::string For::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "FOR:\n";

    strResult += Indent(_depth + 1) + "VARIABLE:\n";
    strResult += m_pVariable->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "CONDITION:\n";
    strResult += m_pCondition->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "EXPRESSION:\n";
    strResult += m_pExpression->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "BLOCK:\n";
    for (auto& pNode : m_vecBlock) {
        strResult += pNode->PrintInfo(_depth + 2);
    }
    return strResult;
}

void For::Interpret()
{
    GET_INTERPRETER().m_listLocalFrame.back().emplace_front();
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
    GET_INTERPRETER().m_listLocalFrame.back().pop_front();
}

std::string If::PrintInfo(int32 _depth)
{
    std::string strResult;
    for (size_t i = 0; i < m_vecCondition.size(); i++) {
        strResult += Indent(_depth) + (i == 0 ? "IF: \n" : "ELIF: \n");
        strResult += Indent(_depth + 1) + "CONDITION: \n";
        strResult += m_vecCondition[i]->PrintInfo(_depth + 2);
        
        strResult += Indent(_depth + 1) + "BLOCK: \n";

        for (auto& pNode : m_vecBlocks[i])
            strResult += pNode->PrintInfo(_depth + 2);
    }
    if (m_vecElseBlock.size() == 0) {
        return std::string();
    }

    strResult += Indent(_depth) + "ELSE: \n";
    for (auto& pNode : m_vecElseBlock) {
        strResult += pNode->PrintInfo(_depth + 1);
    }
    return strResult;
}

void If::Interpret()
{
    for (uint64 i = 0; i < m_vecCondition.size(); i++) {
        auto pResult = m_vecCondition[i]->Interpret();
        if (Object::IsTrue(pResult) == false) {
            continue;
        }
        GET_INTERPRETER().m_listLocalFrame.back().emplace_front();
        for (auto& pNode : m_vecBlocks[i]) {
            pNode->Interpret();
        }
        GET_INTERPRETER().m_listLocalFrame.back().pop_front();
        return;
    }
    if (m_vecElseBlock.empty()) {
        return;
    }
    GET_INTERPRETER().m_listLocalFrame.back().emplace_front();
    for (auto& pNode : m_vecElseBlock) {
        pNode->Interpret();
    }
    GET_INTERPRETER().m_listLocalFrame.back().pop_front();
}

std::string Variable::PrintInfo(int32 _depth)
{
    std::string strResult = 
        Indent(_depth) + "VAR " + m_strName + ": \n";
    if (m_pExpression) {
        // 없을 수도 있지
        strResult += m_pExpression->PrintInfo(_depth + 1);
    }
    return strResult;
}

void Variable::Interpret()
{
    GET_INTERPRETER().m_listLocalFrame.back().front()[m_strName] = m_pExpression->Interpret();
}

std::string Print::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + (m_bLineFeed ? "PRINT_LINE\n" : "PRINT:\n");
    for (auto& pNode : m_vecArgument) {
        strResult += pNode->PrintInfo(_depth + 1);
    }
    return strResult;
}

void Print::Interpret()
{
#ifdef USE_APPLICATION_IMGUI
    for (auto& pNode : m_vecArgument) {
        auto anyValue = pNode->Interpret();
        string strResult = AnyToString(anyValue);
        strResult = std::regex_replace(strResult, std::regex("\\\\n"), "\n");
        ImGui::Text(strResult.c_str());
    }
    if (m_bLineFeed) {
        ImGui::Text("");
    }
#else
    for (auto& pNode : m_vecArgument) {
        auto anyValue = pNode->Interpret();
        std::cout << anyValue;
    }
    if (m_bLineFeed) {
        std::cout << endl;
    }
#endif
}

std::string Return::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "RETURN:\n";
    strResult += m_pExpression->PrintInfo(_depth + 1);
    return strResult;
}

void Return::Interpret()
{
    throw ReturnException{ m_pExpression->Interpret() };
}

std::string Break::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "BREAK\n";
}

void Break::Interpret()
{
    throw BreakException();
}

std::string Continue::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "CONTINUE\n";
}

void Continue::Interpret()
{
    throw ContinueException();
}

std::string ExpressionStatement::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "EXPRESSION:\n";
    strResult += m_pExpression->PrintInfo(_depth + 1);
    return strResult;
}

void ExpressionStatement::Interpret()
{
    m_pExpression->Interpret();
}

std::string Or::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "OR:\n";
    
    strResult += Indent(_depth + 1) + "LHS:\n";
    strResult += m_pLhs->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "RHS:\n";
    strResult += m_pRhs->PrintInfo(_depth + 2);
    return strResult;
}

std::any Or::Interpret()
{
    return Object::IsTrue(m_pLhs->Interpret()) ? true : m_pRhs->Interpret();;
}

std::string And::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "AND:\n";

    strResult += Indent(_depth + 1) + "LHS:\n";
    strResult += m_pLhs->PrintInfo(_depth + 2);

    strResult += Indent(_depth + 1) + "RHS:\n";
    strResult += m_pRhs->PrintInfo(_depth + 2);
    return strResult;
}

std::any And::Interpret()
{
    return Object::IsFalse(m_pLhs->Interpret()) ? false : m_pRhs->Interpret();
}

std::string Relational::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth); 
    strResult += ToString(m_eKind) + ":\n";

    strResult += Indent(_depth + 1); 
    strResult += "LHS:\n";
    strResult += m_pLhs->PrintInfo(_depth + 2);

    strResult += Indent(_depth + 1); 
    strResult += "RHS:\n";
    strResult += m_pRhs->PrintInfo(_depth + 2);
    return strResult;
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

std::string Arithmetic::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + ToString(m_eKind) + ":\n";
    
    strResult += Indent(_depth + 1) + "LHS:\n";
    strResult += m_pLhs->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "RHS:\n";
    strResult += m_pRhs->PrintInfo(_depth + 2);
    return strResult;
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
    if (m_eKind == EKind::Multiply) {
        if (IsFunc(Object::IsNumber)) {
            return Object::ToNumber(lValue) * Object::ToNumber(rValue);
        }
        else if (Object::IsString(lValue) && Object::IsNumber(rValue)) {
            // python 처럼 문자열을 복사한다.
            std::string result;
            std::string temp = Object::ToString(lValue);
            int32 size = Object::ToNumber(rValue);
            result.reserve(temp.length() * size);
            for (int32 i = 0; i < size; ++i) {
                result += temp;
            }
            return result;
        }
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

std::string Unary::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + ToString(m_eKind) + '\n';
    strResult += m_pSub->PrintInfo(_depth + 1);
    return strResult;
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

std::string GetElement::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "GET_ELEMENT:\n";
    
    strResult += Indent(_depth + 1) + "SUB:\n";
    strResult += m_pSub->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "INDEX:\n";
    strResult += m_pIndex->PrintInfo(_depth + 2);
    return strResult;
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

std::string SetElement::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "SET_ELEMENT:\n";
    
    strResult += Indent(_depth + 1) + "SUB:\n";
    strResult += m_pSub->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "INDEX:\n";
    strResult += m_pIndex->PrintInfo(_depth + 2);
    
    strResult += Indent(_depth + 1) + "VALUE:\n";
    strResult += m_pValue->PrintInfo(_depth + 2);
    return strResult;
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

std::string Call::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth); 
    strResult += "CALL:\n";
    
    strResult += Indent(_depth + 1); 
    strResult += "EXPRESSION:\n";
    strResult += m_pSub->PrintInfo(_depth + 2);
    
    for (auto& pNode : m_vecArgument) {
        strResult += Indent(_depth + 1); 
        strResult += "ARGUMENT:\n";
        strResult += pNode->PrintInfo(_depth + 2);
    }
    return strResult;
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
    GET_INTERPRETER().m_listLocalFrame.emplace_back().push_front(mapParameter);
    try {
        Object::ToFunction(value)->Interpret();
    }
    catch (ReturnException exception) {
        GET_INTERPRETER().m_listLocalFrame.pop_back();
        return exception.result;
    }
    GET_INTERPRETER().m_listLocalFrame.pop_back();
    return nullptr;
}

std::string NullLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "null\n";
}

std::any NullLiteral::Interpret()
{
    return nullptr;
}

std::string BooleanLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + (m_bValue ? "true\n" : "false\n");
}

std::any BooleanLiteral::Interpret()
{
    return m_bValue;
}

std::string NumberLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + std::to_string(m_dValue);
}

std::any NumberLiteral::Interpret()
{
    return m_dValue;
}

std::string StringLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "\"" + m_strValue + "\"\n";
}

std::any StringLiteral::Interpret()
{
    return m_strValue;
}

std::string ArrayLiteral::PrintInfo(int32 _depth)
{
    std::string strResult;
    strResult += Indent(_depth) + "[\n";
    for (auto& pNode : m_vecValue) {
        strResult += pNode->PrintInfo(_depth + 1);
    }
    strResult += Indent(_depth) + "]\n";
    return strResult;
}

std::any ArrayLiteral::Interpret()
{
    auto result = std::make_shared<Array>();
    for (auto& pNode : m_vecValue) {
        result->m_vecValue.push_back(pNode->Interpret());
    }
    return result;
}

std::string MapLiteral::PrintInfo(int32 _depth)
{
    std::string strResult;
    strResult += Indent(_depth) + "{\n";
    for (auto& [key, value] : m_mapValue) {
        strResult += key + ": " + value->PrintInfo(_depth + 1);
    }
    strResult += Indent(_depth) + "}\n";
    return strResult;
}

std::any MapLiteral::Interpret()
{
    auto result = std::make_shared<Map>();
    for (auto& [key, value] : m_mapValue) {
        result->m_mapValue.insert_or_assign(key, value->Interpret());
    }
    return result;
}

std::string GetVariable::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "GET_VARIABLE: " + m_strName + "\n";
}

std::any GetVariable::Interpret()
{
    for (auto& vecVariable : GET_INTERPRETER().m_listLocalFrame.back()) {
        if (vecVariable.count(m_strName)) {
            return vecVariable[m_strName];
        }
    }
    if (GET_INTERPRETER().m_mapGlobal.count(m_strName)) {
        return GET_INTERPRETER().m_mapGlobal[m_strName];
    }
    if (GET_INTERPRETER().m_mapFunctionTable.count(m_strName)) {
        return GET_INTERPRETER().m_mapFunctionTable[m_strName];
    }
    if (GET_INTERPRETER().m_mapBuiltinFunctionTable.count(m_strName)) {
        return GET_INTERPRETER().m_mapBuiltinFunctionTable[m_strName];
    }
    return nullptr;
}

std::string SetVariable::PrintInfo(int32 _depth)
{
    std::string strResult;
    strResult += Indent(_depth) + "SET_VARIABLE: " + m_strName + "\n";
    strResult += m_pValue->PrintInfo(_depth + 1);
    return strResult;
}

std::any SetVariable::Interpret()
{
    for (auto& variables : GET_INTERPRETER().m_listLocalFrame.back()) {
        if (variables.count(m_strName)) {
            return variables[m_strName] = m_pValue->Interpret();
        }
    }
    return GET_INTERPRETER().m_mapGlobal[m_strName] = m_pValue->Interpret();
}

std::string Class::PrintInfo(int32 _depth)
{
    std::string strResult =
        Indent(_depth) + "CLASS: " + m_strName + '\n';

    for (auto& pVariable : m_vecVariable)
    {
        if (pVariable.m_pVariable == nullptr) {
            continue;
        }
        strResult += pVariable.m_pVariable->PrintInfo(_depth + 1) + '\n';
    }
    return strResult;
}

void Class::Interpret()
{
    GET_INTERPRETER().m_listLocalFrame.back().front()[m_strName] = *this;
}

std::string SetClassAccess::PrintInfo(int32 _depth)
{
    return std::string();
}

std::any SetClassAccess::Interpret()
{
    return 1;
}

std::string GetClassAccess::PrintInfo(int32 _depth)
{
    return std::string();
}

std::any GetClassAccess::Interpret()
{
    return 1;
}
