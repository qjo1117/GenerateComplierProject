#include "Parser.h"

namespace
{
    static std::vector<CodeToken>::iterator g_current;
}

std::shared_ptr<Program> Parser::Parse(std::vector<CodeToken> _tokens)
{
    auto result = std::make_shared<Program>();
    g_current = _tokens.begin();
    while (g_current->kind != EKind::EndOfToken) {
        switch (g_current->kind) {
        case EKind::Function: {
            result->m_vecFunction.push_back(ParseFunction());
            break;
        }
        default: {
            std::cout << *g_current << " 잘못된 구문입니다.";
            exit(1);
        }
        }
    }
    return result;
}

std::shared_ptr<Function> Parser::ParseFunction()
{
    auto result = std::make_shared<Function>();
    SkipCurrent(EKind::Function);
    result->m_strName = g_current->name;
    SkipCurrent(EKind::Identifier);
    SkipCurrent(EKind::LeftParen);
    if (g_current->kind != EKind::RightParen) {
        do {
            result->m_vecParameter.push_back(g_current->name);
            SkipCurrent(EKind::Identifier);
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightParen);
    SkipCurrent(EKind::LeftBrace);
    result->m_vecBlock = ParseBlock();
    SkipCurrent(EKind::RightBrace);
    return result;
}

std::vector<std::shared_ptr<Statement>> Parser::ParseBlock()
{
    std::vector<std::shared_ptr<Statement>> result;
    while (g_current->kind != EKind::RightBrace) {
        switch (g_current->kind) {
        case EKind::Variable:   result.push_back(ParseVariable());            break;
        case EKind::For:        result.push_back(ParseFor());                 break;
        case EKind::If:         result.push_back(ParseIf());                  break;
        case EKind::Print:
        case EKind::PrintLine:  result.push_back(ParsePrint());               break;
        case EKind::Return:     result.push_back(ParseReturn());              break;
        case EKind::Break:      result.push_back(ParseBreak());               break;
        case EKind::Continue:   result.push_back(ParseContinue());            break;
        case EKind::EndOfToken:
            std::cout << *g_current << " 잘못된 구문입니다.";
            exit(1);
            break;
        default: result.push_back(ParseExpressionStatement()); break;
        }
    }
    return result;
}

std::shared_ptr<Variable> Parser::ParseVariable()
{
    auto result = std::make_shared<Variable>();
    SkipCurrent(EKind::Variable);
    result->m_strName = g_current->name;
    SkipCurrent(EKind::Identifier);
    if (SkipCurrentIf(EKind::Assignment)) {
        result->m_pExpression = ParseExpression();
        if (result->m_pExpression == nullptr) {
            std::cout << "변수 선언에 초기화식이 없습니다.";
            exit(1);
        }
    }
    SkipCurrent(EKind::Semicolon);
    return result;
}

std::shared_ptr<For> Parser::ParseFor()
{
    auto result = std::make_shared<For>();
    SkipCurrent(EKind::For);
    result->m_pVariable = std::make_shared<Variable>();
    result->m_pVariable->m_strName = g_current->name;
    SkipCurrent(EKind::Identifier);
    SkipCurrent(EKind::Assignment);
    result->m_pVariable->m_pExpression = ParseExpression();
    if (result->m_pVariable->m_pExpression == nullptr) {
        std::cout << "for문에 초기화식이 없습니다.";
        exit(1);
    }
    SkipCurrent(EKind::Comma);
    result->m_pCondition = ParseExpression();
    if (result->m_pCondition == nullptr) {
        std::cout << "for문에 조건식이 없습니다.";
        exit(1);
    }
    SkipCurrent(EKind::Comma);
    result->m_pExpression = ParseExpression();
    if (result->m_pExpression == nullptr) {
        std::cout << "for문에 증감식이 없습니다.";
        exit(1);
    }
    SkipCurrent(EKind::LeftBrace);
    result->m_vecBlock = ParseBlock();
    SkipCurrent(EKind::RightBrace);
    return result;
}

std::shared_ptr<If> Parser::ParseIf()
{
    auto result = std::make_shared<If>();
    SkipCurrent(EKind::If);
    do {
        auto condition = ParseExpression();
        if (condition == nullptr) {
            std::cout << "if문에 조건식이 없습니다.";
            exit(1);
        }
        result->conditions.push_back(condition);
        SkipCurrent(EKind::LeftBrace);
        result->blocks.push_back(ParseBlock());
        SkipCurrent(EKind::RightBrace);
    } while (SkipCurrentIf(EKind::ElIf));
    if (SkipCurrentIf(EKind::Else)) {
        SkipCurrent(EKind::LeftBrace);
        result->m_vecElseBlock = ParseBlock();
        SkipCurrent(EKind::RightBrace);
    }
    return result;
}

std::shared_ptr<Print> Parser::ParsePrint()
{
    auto result = std::make_shared<Print>();
    result->lineFeed = g_current->kind == EKind::PrintLine;
    SkipCurrent();
    if (g_current->kind != EKind::Semicolon) {
        do {
            result->m_vecArgument.push_back(ParseExpression());
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::Semicolon);
    return result;
}

std::shared_ptr<Return> Parser::ParseReturn()
{
    auto result = std::make_shared<Return>();
    SkipCurrent(EKind::Return);
    result->m_pExpression = ParseExpression();
    if (result->m_pExpression == nullptr) {
        std::cout << "return문에 식이 없습니다.";
        exit(1);
    }
    SkipCurrent(EKind::Semicolon);
    return result;
}

std::shared_ptr<Break> Parser::ParseBreak()
{
    auto result = std::make_shared<Break>();
    SkipCurrent(EKind::Break);
    SkipCurrent(EKind::Semicolon);
    return result;
}

std::shared_ptr<Continue> Parser::ParseContinue() {
    auto result = std::make_shared<Continue>();
    SkipCurrent(EKind::Continue);
    SkipCurrent(EKind::Semicolon);
    return result;
}

std::shared_ptr<ExpressionStatement> Parser::ParseExpressionStatement()
{
    auto result = std::make_shared<ExpressionStatement>();
    result->m_pExpression = ParseExpression();
    SkipCurrent(EKind::Semicolon);
    return result;
}

std::shared_ptr<Expression> Parser::ParseExpression()
{
    return ParseAssignment();
}

std::shared_ptr<Expression> Parser::ParseAssignment()
{
    auto result = ParseOr();
    if (g_current->kind != EKind::Assignment) {
        return result;
    }

    SkipCurrent(EKind::Assignment);
    if (auto getVariable = std::dynamic_pointer_cast<GetVariable>(result)) {
        auto result = std::make_shared<SetVariable>();
        result->m_strName = getVariable->m_strName;
        result->m_pValue = ParseAssignment();
        return result;
    }
    if (auto getElement = std::dynamic_pointer_cast<GetElement>(result)) {
        auto result = std::make_shared<SetElement>();
        result->m_pSub = getElement->m_pSub;
        result->m_pIndex = getElement->m_pIndex;
        result->m_pValue = ParseAssignment();
        return result;
    }
    std::cout << "잘못된 대입 연산 식입니다.";
    exit(1);
}

std::shared_ptr<Expression> Parser::ParseOr()
{
    auto result = ParseAnd();
    while (SkipCurrentIf(EKind::LogicalOr)) {
        auto temp = std::make_shared<Or>();
        temp->m_pLhs = result;
        temp->m_pRhs = ParseAnd();
        result = temp;
    }
    return result;
}

std::shared_ptr<Expression> Parser::ParseAnd()
{
    auto result = ParseRelational();
    while (SkipCurrentIf(EKind::LogicalAnd)) {
        auto temp = std::make_shared<And>();
        temp->m_pLhs = result;
        temp->m_pRhs = ParseRelational();
        result = temp;
    }
    return result;
}

std::shared_ptr<Expression> Parser::ParseRelational()
{
    static std::set<EKind> operators = {
      EKind::Equal,
      EKind::NotEqual,
      EKind::LessThan,
      EKind::GreaterThan,
      EKind::LessOrEqual,
      EKind::GreaterOrEqual,
    };
    auto result = ParseArithmetic1();
    while (operators.count(g_current->kind)) {
        auto temp = std::make_shared<Relational>();
        temp->m_eKind = g_current->kind;
        SkipCurrent();
        temp->m_pLhs = result;
        temp->m_pRhs = ParseArithmetic1();
        result = temp;
    }
    return result;
}

std::shared_ptr<Expression> Parser::ParseArithmetic1() {
    static std::set<EKind> operators = {
      EKind::Add,
      EKind::Subtract
    };
    auto result = ParseArithmetic2();
    while (operators.count(g_current->kind)) {
        auto temp = std::make_shared<Arithmetic>();
        temp->m_eKind = g_current->kind;
        SkipCurrent();
        temp->m_pLhs = result;
        temp->m_pRhs = ParseArithmetic2();
        result = temp;
    }
    return result;
}

std::shared_ptr<Expression> Parser::ParseArithmetic2()
{
    static std::set<EKind> operators = {
      EKind::Multiply,
      EKind::Divide,
      EKind::Modulo,
    };
    auto result = ParseUnary();
    while (operators.count(g_current->kind)) {
        auto temp = std::make_shared<Arithmetic>();
        temp->m_eKind = g_current->kind;
        SkipCurrent();
        temp->m_pLhs = result;
        temp->m_pRhs = ParseUnary();
        result = temp;
    }
    return result;
}

std::shared_ptr<Expression> Parser::ParseUnary()
{
    static std::set<EKind> operators = {
      EKind::Add,
      EKind::Subtract,
    };
    while (operators.count(g_current->kind)) {
        auto result = std::make_shared<Unary>();
        result->m_eKind = g_current->kind;
        SkipCurrent();
        result->m_pSub = ParseUnary();
        return result;
    }
    return ParseOperand();
}

std::shared_ptr<Expression> Parser::ParseOperand()
{
    std::shared_ptr<Expression> result = nullptr;
    switch (g_current->kind) {
    case EKind::NullLiteral:   result = ParseNullLiteral();      break;
    case EKind::TrueLiteral:
    case EKind::FalseLiteral:  result = ParseBooleanLiteral();   break;
    case EKind::NumberLiteral: result = ParseNumberLiteral();    break;
    case EKind::StringLiteral: result = ParseStringLiteral();    break;
    case EKind::LeftBraket:    result = ParseListLiteral();      break;
    case EKind::LeftBrace:     result = ParseMapLiteral();       break;
    case EKind::Identifier:    result = ParseIdentifier();       break;
    case EKind::LeftParen:     result = ParseInnerExpression();  break;
    default:                  std::cout << "잘못된 식입니다.";       exit(1);
    }
    return ParsePostfix(result);
}

std::shared_ptr<Expression> Parser::ParseNullLiteral()
{
    SkipCurrent(EKind::NullLiteral);
    auto result = std::make_shared<NullLiteral>();
    return result;
}

std::shared_ptr<Expression> Parser::ParseBooleanLiteral()
{
    auto result = std::make_shared<BooleanLiteral>();
    result->m_bValue = g_current->kind == EKind::TrueLiteral;
    SkipCurrent();
    return result;
}

std::shared_ptr<Expression> Parser::ParseNumberLiteral()
{
    auto result = std::make_shared<NumberLiteral>();
    result->m_dValue = std::stod(g_current->name);
    SkipCurrent(EKind::NumberLiteral);
    return result;
}

std::shared_ptr<Expression> Parser::ParseStringLiteral()
{
    auto result = std::make_shared<StringLiteral>();
    result->m_strValue = g_current->name;
    SkipCurrent(EKind::StringLiteral);
    return result;
}

std::shared_ptr<Expression> Parser::ParseListLiteral()
{
    auto result = std::make_shared<ArrayLiteral>();
    SkipCurrent(EKind::LeftBraket);
    if (g_current->kind != EKind::RightBraket) {
        do
        {
            result->m_vecValue.push_back(ParseExpression());
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightBraket);
    return result;
}

std::shared_ptr<Expression> Parser::ParseMapLiteral()
{
    auto result = std::make_shared<MapLiteral>();
    SkipCurrent(EKind::LeftBrace);
    if (g_current->kind != EKind::RightBrace) {
        do {
            auto name = g_current->name;
            SkipCurrent(EKind::StringLiteral);
            SkipCurrent(EKind::Colon);
            auto value = ParseExpression();
            result->m_mapValue[name] = value;
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightBrace);
    return result;
}

std::shared_ptr<Expression> Parser::ParseIdentifier()
{
    auto result = std::make_shared<GetVariable>();
    result->m_strName = g_current->name;
    SkipCurrent(EKind::Identifier);
    return result;
}

std::shared_ptr<Expression> Parser::ParseInnerExpression()
{
    SkipCurrent(EKind::LeftParen);
    auto result = ParseExpression();
    SkipCurrent(EKind::RightParen);
    return result;
}

std::shared_ptr<Expression> Parser::ParsePostfix(std::shared_ptr<Expression> _pSub)
{
    while (true) {
        switch (g_current->kind) {
        case EKind::LeftParen:  _pSub = ParseCall(_pSub);    break;
        case EKind::LeftBraket: _pSub = ParseElement(_pSub); break;
        default: return _pSub;
        }
    }
}

std::shared_ptr<Expression> Parser::ParseCall(std::shared_ptr<Expression> _pSub)
{
    auto result = std::make_shared<Call>();
    result->m_pSub = _pSub;
    SkipCurrent(EKind::LeftParen);
    if (g_current->kind != EKind::RightParen) {
        do result->m_vecArgument.push_back(ParseExpression());
        while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightParen);
    return result;
}

std::shared_ptr<Expression> Parser::ParseElement(std::shared_ptr<Expression> _pSub)
{
    auto result = std::make_shared<GetElement>();
    result->m_pSub = _pSub;
    SkipCurrent(EKind::LeftBraket);
    result->m_pIndex = ParseExpression();
    SkipCurrent(EKind::RightBraket);
    return result;
}

void Parser::SkipCurrent()
{
    g_current++;
}

void Parser::SkipCurrent(EKind _eKind)
{
    if (g_current->kind != _eKind) {
        std::cout << ToString(_eKind) + " 토큰이 필요합니다.\n" << "Row : " << g_current->row << " Col : " << g_current->col;
        exit(1);
    }
    g_current++;
}

bool Parser::SkipCurrentIf(EKind _eKind)
{
    if (g_current->kind != _eKind) {
        return false;
    }
    g_current++;
    return true;
}
