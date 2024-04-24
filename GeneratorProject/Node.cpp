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
    InterpreterMgr.m_mapFunctionTable.clear();
    InterpreterMgr.m_mapGlobal.clear();
    InterpreterMgr.m_listLocalFrame.clear();
    for (auto& node : _pProgram->m_vecFunction) {
        InterpreterMgr.m_mapFunctionTable[node->m_strName] = node;
    }
    if (InterpreterMgr.m_mapFunctionTable["main"] == nullptr) {
        std::cout << "Cannot find main function\n";
        return;
    }
    InterpreterMgr.m_listLocalFrame.emplace_back().emplace_front();
    try {
        InterpreterMgr.m_mapFunctionTable["main"]->Interpret();
    }
    catch (ReturnException) {}
    catch (BreakException) {}
    catch (ContinueException) {}
    InterpreterMgr.m_listLocalFrame.pop_back();
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

std::tuple<std::vector<Code>, std::map<std::string, std::size_t>>
        Generater::Generate(std::shared_ptr<Program> _pProgram)
{
    m_vecCodeList.clear();
    m_mapFunctionTable.clear();
    WriteCode(Instruction::GetGlobal, string("main"));
    WriteCode(Instruction::Call, static_cast<size_t>(0));
    WriteCode(Instruction::Exit);
    for (auto& pNode : _pProgram->m_vecFunction) {
        pNode->Generate();
    }
    return { m_vecCodeList, m_mapFunctionTable };
}

void Generater::SetLocal(std::string _strLocal)
{
    m_listSymbolStackTable.front()[_strLocal] = m_vecOffsetStack.back();
    m_vecOffsetStack.back() += 1;
    m_iLocalSize = max(m_iLocalSize, m_vecOffsetStack.back());
}

uint64 Generater::GetLocal(std::string _strLocal)
{
    for (auto& symbolTable : m_listSymbolStackTable) {
        if (symbolTable.count(_strLocal)) {
            return symbolTable[_strLocal];
        }
    }
    return SIZE_MAX;
}

void Generater::InitBlock()
{
    m_iLocalSize = 0;
    m_vecOffsetStack.push_back(0);
    m_listSymbolStackTable.emplace_front();
}

void Generater::PushBlock()
{
    m_listSymbolStackTable.emplace_front();
    m_vecOffsetStack.push_back(m_vecOffsetStack.back());
}

void Generater::PopBlock()
{
    m_vecOffsetStack.pop_back();
    m_listSymbolStackTable.pop_front();
}

uint64 Generater::WriteCode(Instruction _instruction)
{
    m_vecCodeList.push_back({ _instruction });
    return m_vecCodeList.size() - 1;
}

uint64 Generater::WriteCode(Instruction _instruction, std::any _anyValue)
{
    m_vecCodeList.push_back({ _instruction, _anyValue });
    return m_vecCodeList.size() - 1;
}

void Generater::PatchAddress(uint64 _codeIndex)
{
    m_vecCodeList[_codeIndex].m_anyOperand = m_vecCodeList.size();
}

void Generater::PatchOperand(uint64 _codeIndex, uint64 _operand)
{
    m_vecCodeList[_codeIndex].m_anyOperand = _operand;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Function
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

void Function::Generate()
{
    GeneraterMgr.m_mapFunctionTable[m_strName] = GeneraterMgr.m_vecCodeList.size();
    auto temp = GeneraterMgr.WriteCode(Instruction::Alloca);
    GeneraterMgr.InitBlock();
    for (std::string& paramName : m_vecParameter) {
        GeneraterMgr.SetLocal(paramName);
    }
    for (auto& pScope : m_vecBlock) {
        pScope->Generate();
    }
    GeneraterMgr.PopBlock();
    GeneraterMgr.PatchOperand(temp, GeneraterMgr.m_iLocalSize);
    GeneraterMgr.WriteCode(Instruction::Return);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - For
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
    InterpreterMgr.m_listLocalFrame.back().emplace_front();
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
    InterpreterMgr.m_listLocalFrame.back().pop_front();
}

void For::Generate()
{
    GeneraterMgr.m_vecBreakStack.emplace_back();
    GeneraterMgr.m_vecContinueStack.emplace_back();

    GeneraterMgr.PushBlock();
    m_pVariable->Generate();
    uint64 jumpAddress = GeneraterMgr.m_vecCodeList.size();
    m_pCondition->Generate();
    uint64 conditionJump = GeneraterMgr.WriteCode(Instruction::ConditionJump);

    for (auto& pNode : m_vecBlock) {
        pNode->Generate();
    }

    uint64 continueAddress = GeneraterMgr.m_vecCodeList.size();
    m_pExpression->Generate();
    GeneraterMgr.WriteCode(Instruction::PopOperand);
    GeneraterMgr.WriteCode(Instruction::Jump, jumpAddress);
    GeneraterMgr.PatchAddress(conditionJump);
    GeneraterMgr.PopBlock();

    for (uint64 jump : GeneraterMgr.m_vecContinueStack.back()) {
        GeneraterMgr.PatchOperand(jump, continueAddress);
    }
    GeneraterMgr.m_vecContinueStack.pop_back();

    for (uint64 jump : GeneraterMgr.m_vecBreakStack.back()) {
        GeneraterMgr.PatchAddress(jump);
    }
    GeneraterMgr.m_vecBreakStack.pop_back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - If
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
        InterpreterMgr.m_listLocalFrame.back().emplace_front();
        for (auto& pNode : m_vecBlocks[i]) {
            pNode->Interpret();
        }
        InterpreterMgr.m_listLocalFrame.back().pop_front();
        return;
    }
    if (m_vecElseBlock.empty()) {
        return;
    }
    InterpreterMgr.m_listLocalFrame.back().emplace_front();
    for (auto& pNode : m_vecElseBlock) {
        pNode->Interpret();
    }
    InterpreterMgr.m_listLocalFrame.back().pop_front();
}

void If::Generate()
{
    std::vector<uint64> vecJumpList;
    for (uint32 i = 0; i < m_vecCondition.size(); ++i) {
        m_vecCondition[i]->Generate();
        auto conditionJump = GeneraterMgr.WriteCode(Instruction::ConditionJump);
        GeneraterMgr.PushBlock();
        for (auto& pNode : m_vecBlocks[i]) {
            pNode->Generate();
        }
        GeneraterMgr.PopBlock();
        vecJumpList.push_back(GeneraterMgr.WriteCode(Instruction::Jump));
        GeneraterMgr.PatchAddress(conditionJump);
    }

    if (m_vecElseBlock.empty() == false) {
        GeneraterMgr.PushBlock();
        for (auto& pNode : m_vecElseBlock) {
            pNode->Generate();
        }
        GeneraterMgr.PopBlock();
    }
    for (uint64& jump : vecJumpList) {
        GeneraterMgr.PatchAddress(jump);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Variable
std::string Variable::PrintInfo(int32 _depth)
{
    std::string strResult = 
        Indent(_depth) + "VAR " + m_strName + ": ";
    if (m_pExpression) {
        // 없을 수도 있지
        strResult += m_pExpression->PrintInfo(_depth + 1) + '\n';
    }
    return strResult;
}

void Variable::Interpret()
{
    InterpreterMgr.m_listLocalFrame.back().front()[m_strName] = m_pExpression->Interpret();
}

void Variable::Generate()
{
    GeneraterMgr.SetLocal(m_strName);
    m_pExpression->Generate();
    GeneraterMgr.WriteCode(Instruction::SetLocal, GeneraterMgr.GetLocal(m_strName));
    GeneraterMgr.WriteCode(Instruction::PopOperand);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Print
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

void Print::Generate()
{
    for (uint64 i = m_vecArgument.size(); i > 0; --i) {
        m_vecArgument[i - 1]->Generate();
    }
    GeneraterMgr.WriteCode(Instruction::Print, m_vecArgument.size());
    if (m_bLineFeed) {
        GeneraterMgr.WriteCode(Instruction::PrintLine);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Return
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

void Return::Generate()
{
    m_pExpression->Generate();
    GeneraterMgr.WriteCode(Instruction::Return);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Break
std::string Break::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "BREAK\n";
}

void Break::Interpret()
{
    throw BreakException();
}

void Break::Generate()
{
    if (GeneraterMgr.m_vecBreakStack.empty()) {
        return;
    }
    uint64 jump = GeneraterMgr.WriteCode(Instruction::Jump);
    GeneraterMgr.m_vecBreakStack.back().push_back(jump);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Continue
std::string Continue::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "CONTINUE\n";
}

void Continue::Interpret()
{
    throw ContinueException();
}

void Continue::Generate()
{
    if (GeneraterMgr.m_vecContinueStack.empty()) {
        return;
    }
    uint64 jump = GeneraterMgr.WriteCode(Instruction::Jump);
    GeneraterMgr.m_vecContinueStack.back().push_back(jump);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - ExpressionStatement
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

void ExpressionStatement::Generate()
{
    m_pExpression->Generate();
    GeneraterMgr.WriteCode(Instruction::PopOperand);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Or
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

void Or::Generate()
{
    m_pLhs->Generate();
    uint64 logicalOr = GeneraterMgr.WriteCode(Instruction::LogicalOr);
    m_pRhs->Generate();
    GeneraterMgr.PatchAddress(logicalOr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - And
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

void And::Generate()
{
    m_pLhs->Generate();
    uint64 logicalAnd = GeneraterMgr.WriteCode(Instruction::LogicalAnd);
    m_pRhs->Generate();
    GeneraterMgr.PatchAddress(logicalAnd);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Relational
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
    auto IsCrossFunc = [&lValue, &rValue](std::function<bool(std::any)> _isFunc1, std::function<bool(std::any)> _isFunc2) {
        if (_isFunc1(lValue) && _isFunc2(rValue)) {
            return 1;
        }
        if (_isFunc2(lValue) && _isFunc1(rValue)) {
            return 2;
        }
        return 0;
    };

    if (m_eKind == EKind::Equal) {
        if (IsFunc(Object::IsNull)) {
            return true;
        }
        if (IsFunc(Object::IsBoolean)) {
            return Object::ToBoolean(lValue) == Object::ToBoolean(rValue);
        }
        if (IsFunc(Object::IsNumber)) {
            return Object::ToNumber(lValue) == Object::ToNumber(rValue);
        }
        if (IsFunc(Object::IsFloat)) {
            return Object::ToFloat(lValue) == Object::ToFloat(rValue);
        }
        if (IsFunc(Object::IsString)) {
            return Object::ToString(lValue) == Object::ToString(rValue);
        }

        switch (int32 ret = IsCrossFunc(Object::IsNumber, Object::IsFloat)) {
            case 1: return Object::ToNumber(lValue) == Object::ToFloat(rValue);
            case 2: return Object::ToFloat(lValue) == Object::ToNumber(rValue);
        }

        switch (int32 ret = IsCrossFunc(Object::IsNumber, Object::IsString)) {
            case 1: return std::to_string(Object::ToNumber(lValue)) == Object::ToString(rValue);
            case 2: return Object::ToString(lValue) == std::to_string(Object::ToNumber(rValue));
        }
    }

    if (m_eKind == EKind::NotEqual) {
        if (IsFunc(Object::IsNull)) {
            return false;
        }
        if (Object::IsNull(lValue) || Object::IsNull(rValue)) {
            return true;
        }
        if (IsFunc(Object::IsBoolean)) {
            return Object::ToBoolean(lValue) != Object::ToBoolean(rValue);
        }
        if (IsFunc(Object::IsNumber)) {
            return Object::ToNumber(lValue) != Object::ToNumber(rValue);
        }
        if (IsFunc(Object::IsString)) {
            return Object::ToString(lValue) != Object::ToString(rValue);
        }

        switch (int32 ret = IsCrossFunc(Object::IsNumber, Object::IsFloat)) {
        case 1: return Object::ToNumber(lValue) != Object::ToFloat(rValue);
        case 2: return Object::ToFloat(lValue) != Object::ToNumber(rValue);
        }

        switch (int32 ret = IsCrossFunc(Object::IsNumber, Object::IsString)) {
        case 1: return std::to_string(Object::ToNumber(lValue)) != Object::ToString(rValue);
        case 2: return Object::ToString(lValue) != std::to_string(Object::ToNumber(rValue));
        }
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

void Relational::Generate()
{
    static std::map<EKind, Instruction> mapKindToInstructionTable = {
        { EKind::Equal,             Instruction::Equal },   
        { EKind::NotEqual,          Instruction::NotEqual },
        { EKind::LessThan,          Instruction::LessThan },
        { EKind::GreaterThan,       Instruction::GreaterThan },
        { EKind::LessOrEqual,       Instruction::LessOrEqual },
        { EKind::GreaterOrEqual,    Instruction::GreaterOrEqual },
    };

    m_pLhs->Generate();
    m_pRhs->Generate();
    GeneraterMgr.WriteCode(mapKindToInstructionTable[m_eKind]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Arithmetic
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
            uint64 size = Object::ToNumber(rValue);
            result.reserve(temp.length() * size);
            for (uint64 i = 0; i < size; ++i) {
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

void Arithmetic::Generate()
{
    static std::map<EKind, Instruction> mapKindToInstructionTable = {
        { EKind::Add,           Instruction::Add },   
        { EKind::Subtract,      Instruction::Subtract },
        { EKind::Multiply,      Instruction::Multiply },
        { EKind::Divide,        Instruction::Divide },
        { EKind::Modulo,        Instruction::Modulo },
    };
    m_pLhs->Generate();
    m_pRhs->Generate();
    GeneraterMgr.WriteCode(mapKindToInstructionTable[m_eKind]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Unary
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
        return std::abs((long)Object::ToNumber(value));
    }
    if (m_eKind == EKind::Subtract && Object::IsNumber(value)) {
        return Object::ToNumber(value) * -1;
    }
    return 0.0;
}

void Unary::Generate()
{
    std::map<EKind, Instruction> mapKindToInstructionTable = {
        { EKind::Add,       Instruction::Absolute },  
        { EKind::Subtract,  Instruction::ReverseSign },
    };
    m_pSub->Generate();
    GeneraterMgr.WriteCode(mapKindToInstructionTable[m_eKind]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - GetElement
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

void GetElement::Generate()
{
    m_pSub->Generate();
    m_pIndex->Generate();
    GeneraterMgr.WriteCode(Instruction::GetElement);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - SetElement
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

void SetElement::Generate()
{
    m_pValue->Generate();
    m_pSub->Generate();
    m_pIndex->Generate();
    GeneraterMgr.WriteCode(Instruction::SetElement);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Call
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
    InterpreterMgr.m_listLocalFrame.emplace_back().push_front(mapParameter);
    try {
        Object::ToFunction(value)->Interpret();
    }
    catch (ReturnException exception) {
        InterpreterMgr.m_listLocalFrame.pop_back();
        return exception.result;
    }
    InterpreterMgr.m_listLocalFrame.pop_back();
    return nullptr;
}

void Call::Generate()
{
    for (uint64 i = m_vecArgument.size(); i > 0; --i) {
        m_vecArgument[i - 1]->Generate();
    }
    m_pSub->Generate();
    GeneraterMgr.WriteCode(Instruction::Call, m_vecArgument.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - NullLiteral
std::string NullLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "null\n";
}

std::any NullLiteral::Interpret()
{
    return nullptr;
}

void NullLiteral::Generate()
{
    GeneraterMgr.WriteCode(Instruction::PushNull);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - BooleanLiteral
std::string BooleanLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + (m_bValue ? "true\n" : "false\n");
}

std::any BooleanLiteral::Interpret()
{
    return m_bValue;
}

void BooleanLiteral::Generate()
{
    GeneraterMgr.WriteCode(Instruction::PushBoolean);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - NumberLiteral
std::string NumberLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + std::to_string(m_uValue);
}

std::any NumberLiteral::Interpret()
{
    return m_uValue;
}

void NumberLiteral::Generate()
{
    GeneraterMgr.WriteCode(Instruction::PushNumber, m_uValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - FloatLiteral
std::string FloatLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + std::to_string(m_dValue);
}

std::any FloatLiteral::Interpret()
{
    return m_dValue;
}

void FloatLiteral::Generate()
{
    GeneraterMgr.WriteCode(Instruction::PushNumber, m_dValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - StringLiteral
std::string StringLiteral::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "\"" + m_strValue + "\"\n";
}

std::any StringLiteral::Interpret()
{
    return m_strValue;
}

void StringLiteral::Generate()
{
    GeneraterMgr.WriteCode(Instruction::PushString, m_strValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - ArrayLiteral
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

void ArrayLiteral::Generate()
{
    for (uint64 i = m_vecValue.size(); i > 0; --i) {
        m_vecValue[i - 1]->Generate();
    }
    GeneraterMgr.WriteCode(Instruction::PushArray, m_vecValue.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - MapLiteral
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

void MapLiteral::Generate()
{
    for (auto& [key, pValue] : m_mapValue) {
        GeneraterMgr.WriteCode(Instruction::PushString, key);
        pValue->Generate();
    }
    GeneraterMgr.WriteCode(Instruction::PushMap, m_mapValue.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - GetVariable
std::string GetVariable::PrintInfo(int32 _depth)
{
    return Indent(_depth) + "GET_VARIABLE: " + m_strName + "\n";
}

std::any GetVariable::Interpret()
{
    for (auto& vecVariable : InterpreterMgr.m_listLocalFrame.back()) {
        if (vecVariable.count(m_strName)) {
            return vecVariable[m_strName];
        }
    }
    if (InterpreterMgr.m_mapGlobal.count(m_strName)) {
        return InterpreterMgr.m_mapGlobal[m_strName];
    }
    if (InterpreterMgr.m_mapFunctionTable.count(m_strName)) {
        return InterpreterMgr.m_mapFunctionTable[m_strName];
    }
    if (InterpreterMgr.m_mapBuiltinFunctionTable.count(m_strName)) {
        return InterpreterMgr.m_mapBuiltinFunctionTable[m_strName];
    }
    return nullptr;
}

void GetVariable::Generate()
{
    if (GeneraterMgr.GetLocal(m_strName) == SIZE_MAX) {
        GeneraterMgr.WriteCode(Instruction::GetGlobal, m_strName);
    }
    else {
        GeneraterMgr.WriteCode(Instruction::GetLocal, GeneraterMgr.GetLocal(m_strName));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - SetVariable
std::string SetVariable::PrintInfo(int32 _depth)
{
    std::string strResult;
    strResult += Indent(_depth) + "SET_VARIABLE: " + m_strName + "\n";
    strResult += m_pValue->PrintInfo(_depth + 1);
    return strResult;
}

std::any SetVariable::Interpret()
{
    for (auto& variables : InterpreterMgr.m_listLocalFrame.back()) {
        if (variables.count(m_strName)) {
            return variables[m_strName] = m_pValue->Interpret();
        }
    }
    return InterpreterMgr.m_mapGlobal[m_strName] = m_pValue->Interpret();
}

void SetVariable::Generate()
{
    m_pValue->Generate();
    if (GeneraterMgr.GetLocal(m_strName) == SIZE_MAX) {
        GeneraterMgr.WriteCode(Instruction::SetGlobal, m_strName);
    }
    else {
        GeneraterMgr.WriteCode(Instruction::SetLocal, GeneraterMgr.GetLocal(m_strName));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - Class
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
    for (auto& pVariable : m_vecVariable)
    {
        if (pVariable.m_pVariable->m_pExpression)
        {
            // var형식이기 때문에 초기값이라도 있어야 정의되도록하게 만든다.
            throw;
            continue;
        }
        auto tupleTemp = std::make_tuple(pVariable.m_eAccess, pVariable.m_pVariable->m_pExpression->Interpret());
        InterpreterMgr.m_mapClassDefaultTable[m_strName].push_back(std::move(tupleTemp));
    }
}

void Class::Generate()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - SetClassAccess
std::string SetClassAccess::PrintInfo(int32 _depth)
{
    return std::string();
}

std::any SetClassAccess::Interpret()
{
    return 1;
}

void SetClassAccess::Generate()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - GetClassAccess
std::string GetClassAccess::PrintInfo(int32 _depth)
{
    std::string strResult = 
        Indent(_depth) + "GET_CLASS_ACCESS:" + m_pSub->PrintInfo(_depth) + '\n';
    strResult += m_pMember->PrintInfo(_depth + 1) + '\n';
    return strResult;
}

std::any GetClassAccess::Interpret()
{
    return m_pSub->Interpret();
}

void GetClassAccess::Generate()
{
}