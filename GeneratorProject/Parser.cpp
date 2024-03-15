#include "Parser.h"

namespace
{
    static std::vector<CodeToken>::iterator g_current;
}

std::shared_ptr<Program> Parser::Parse(std::vector<CodeToken> _tokens)
{
    auto pResult = std::make_shared<Program>();
    g_current = _tokens.begin();
    while (g_current->m_eKind != EKind::EndOfToken) {
        switch (g_current->m_eKind) {
        case EKind::Function: {
            pResult->m_vecFunction.push_back(ParseFunction());
            break;
        case EKind::Class:
            pResult->m_vecClass.push_back(ParseClass());
            break;
        }
        default: {
            throw;
        }
        }
    }
    return pResult;
}

std::shared_ptr<Function> Parser::ParseFunction()
{
    auto pResult = std::make_shared<Function>();
    SkipCurrent(EKind::Function);
    pResult->m_strName = g_current->m_strName;
    SkipCurrent(EKind::Identifier);
    SkipCurrent(EKind::LeftParen);
    if (g_current->m_eKind != EKind::RightParen) {
        do {
            pResult->m_vecParameter.push_back(g_current->m_strName);
            SkipCurrent(EKind::Identifier);
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightParen);
    SkipCurrent(EKind::LeftBrace);
    pResult->m_vecBlock = ParseBlock();
    SkipCurrent(EKind::RightBrace);
    return pResult;
}

std::vector<std::shared_ptr<Statement>> Parser::ParseBlock()
{
    std::vector<std::shared_ptr<Statement>> pResult;
    while (g_current->m_eKind != EKind::RightBrace) {
        switch (g_current->m_eKind) {
        case EKind::Variable:   pResult.push_back(ParseVariable());            break;
        case EKind::For:        pResult.push_back(ParseFor());                 break;
        case EKind::If:         pResult.push_back(ParseIf());                  break;
        case EKind::Print:
        case EKind::PrintLine:  pResult.push_back(ParsePrint());               break;
        case EKind::Return:     pResult.push_back(ParseReturn());              break;
        case EKind::Break:      pResult.push_back(ParseBreak());               break;
        case EKind::Continue:   pResult.push_back(ParseContinue());            break;
        case EKind::EndOfToken:
            throw;
            break;
        default: pResult.push_back(ParseExpressionStatement()); break;
        }
    }
    return pResult;
}

std::shared_ptr<Variable> Parser::ParseVariable()
{
    auto pResult = std::make_shared<Variable>();
    SkipCurrent(EKind::Variable);
    pResult->m_strName = g_current->m_strName;
    SkipCurrent(EKind::Identifier);
    if (SkipCurrentIf(EKind::Assignment)) {
        pResult->m_pExpression = ParseExpression();
        if (pResult->m_pExpression == nullptr) {
            std::cout << "변수 선언에 초기화식이 없습니다.";
            throw;
        }
    }
    SkipCurrent(EKind::Semicolon);
    return pResult;
}

std::shared_ptr<For> Parser::ParseFor()
{
    auto pResult = std::make_shared<For>();
    SkipCurrent(EKind::For);
    pResult->m_pVariable = std::make_shared<Variable>();
    pResult->m_pVariable->m_strName = g_current->m_strName;
    SkipCurrent(EKind::Identifier);
    SkipCurrent(EKind::Assignment);
    pResult->m_pVariable->m_pExpression = ParseExpression();
    if (pResult->m_pVariable->m_pExpression == nullptr) {
        std::cout << "for문에 초기화식이 없습니다.";
        throw;
    }
    SkipCurrent(EKind::Comma);
    pResult->m_pCondition = ParseExpression();
    if (pResult->m_pCondition == nullptr) {
        std::cout << "for문에 조건식이 없습니다.";
        throw;
    }
    SkipCurrent(EKind::Comma);
    pResult->m_pExpression = ParseExpression();
    if (pResult->m_pExpression == nullptr) {
        std::cout << "for문에 증감식이 없습니다.";
        throw;
    }
    SkipCurrent(EKind::LeftBrace);
    pResult->m_vecBlock = ParseBlock();
    SkipCurrent(EKind::RightBrace);
    return pResult;
}

std::shared_ptr<If> Parser::ParseIf()
{
    auto pResult = std::make_shared<If>();
    SkipCurrent(EKind::If);
    do {
        auto condition = ParseExpression();
        if (condition == nullptr) {
            std::cout << "if문에 조건식이 없습니다.";
            throw;
        }
        pResult->m_vecCondition.push_back(condition);
        SkipCurrent(EKind::LeftBrace);
        pResult->m_vecBlocks.push_back(ParseBlock());
        SkipCurrent(EKind::RightBrace);
    } while (SkipCurrentIf(EKind::ElIf));
    if (SkipCurrentIf(EKind::Else)) {
        SkipCurrent(EKind::LeftBrace);
        pResult->m_vecElseBlock = ParseBlock();
        SkipCurrent(EKind::RightBrace);
    }
    return pResult;
}

std::shared_ptr<Print> Parser::ParsePrint()
{
    auto pResult = std::make_shared<Print>();
    pResult->m_bLineFeed = g_current->m_eKind == EKind::PrintLine;
    SkipCurrent();
    if (g_current->m_eKind != EKind::Semicolon) {
        do {
            pResult->m_vecArgument.push_back(ParseExpression());
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::Semicolon);
    return pResult;
}

std::shared_ptr<Return> Parser::ParseReturn()
{
    auto pResult = std::make_shared<Return>();
    SkipCurrent(EKind::Return);
    pResult->m_pExpression = ParseExpression();
    if (pResult->m_pExpression == nullptr) {
        std::cout << "return문에 식이 없습니다.";
        throw;
    }
    SkipCurrent(EKind::Semicolon);
    return pResult;
}

std::shared_ptr<Break> Parser::ParseBreak()
{
    auto pResult = std::make_shared<Break>();
    SkipCurrent(EKind::Break);
    SkipCurrent(EKind::Semicolon);
    return pResult;
}

std::shared_ptr<Continue> Parser::ParseContinue() {
    auto pResult = std::make_shared<Continue>();
    SkipCurrent(EKind::Continue);
    SkipCurrent(EKind::Semicolon);
    return pResult;
}

std::shared_ptr<ExpressionStatement> Parser::ParseExpressionStatement()
{
    auto pResult = std::make_shared<ExpressionStatement>();
    pResult->m_pExpression = ParseExpression();
    SkipCurrent(EKind::Semicolon);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseExpression()
{
    return ParseAssignment();
}

std::shared_ptr<Expression> Parser::ParseAssignment()
{
    auto pResult = ParseOr();
    if (g_current->m_eKind != EKind::Assignment) {
        return pResult;
    }

    SkipCurrent(EKind::Assignment);
    if (auto getVariable = std::dynamic_pointer_cast<GetVariable>(pResult)) {
        auto pResult = std::make_shared<SetVariable>();
        pResult->m_strName = getVariable->m_strName;
        pResult->m_pValue = ParseAssignment();
        return pResult;
    }
    if (auto getElement = std::dynamic_pointer_cast<GetElement>(pResult)) {
        auto pResult = std::make_shared<SetElement>();
        pResult->m_pSub = getElement->m_pSub;
        pResult->m_pIndex = getElement->m_pIndex;
        pResult->m_pValue = ParseAssignment();
        return pResult;
    }
    if (auto pGetClassAccess = std::dynamic_pointer_cast<GetClassAccess>(pResult)) {
        auto pResult = std::make_shared<SetClassAccess>();
        pResult->m_pSub = pGetClassAccess->m_pSub;
        pResult->m_pMember = pGetClassAccess->m_pMember;
        pResult->m_pValue = ParseAssignment();
    }
    std::cout << "잘못된 대입 연산 식입니다.";
    throw;
}

std::shared_ptr<Expression> Parser::ParseOr()
{
    auto pResult = ParseAnd();
    while (SkipCurrentIf(EKind::LogicalOr)) {
        auto temp = std::make_shared<Or>();
        temp->m_pLhs = pResult;
        temp->m_pRhs = ParseAnd();
        pResult = temp;
    }
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseAnd()
{
    auto pResult = ParseRelational();
    while (SkipCurrentIf(EKind::LogicalAnd)) {
        auto temp = std::make_shared<And>();
        temp->m_pLhs = pResult;
        temp->m_pRhs = ParseRelational();
        pResult = temp;
    }
    return pResult;
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
    auto pResult = ParseArithmetic1();
    while (operators.count(g_current->m_eKind)) {
        auto temp = std::make_shared<Relational>();
        temp->m_eKind = g_current->m_eKind;
        SkipCurrent();
        temp->m_pLhs = pResult;
        temp->m_pRhs = ParseArithmetic1();
        pResult = temp;
    }
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseArithmetic1() {
    static std::set<EKind> operators = {
      EKind::Add,
      EKind::Subtract
    };
    auto pResult = ParseArithmetic2();
    while (operators.count(g_current->m_eKind)) {
        auto temp = std::make_shared<Arithmetic>();
        temp->m_eKind = g_current->m_eKind;
        SkipCurrent();
        temp->m_pLhs = pResult;
        temp->m_pRhs = ParseArithmetic2();
        pResult = temp;
    }
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseArithmetic2()
{
    static std::set<EKind> operators = {
      EKind::Multiply,
      EKind::Divide,
      EKind::Modulo,
    };
    auto pResult = ParseUnary();
    while (operators.count(g_current->m_eKind)) {
        auto temp = std::make_shared<Arithmetic>();
        temp->m_eKind = g_current->m_eKind;
        SkipCurrent();
        temp->m_pLhs = pResult;
        temp->m_pRhs = ParseUnary();
        pResult = temp;
    }
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseUnary()
{
    static std::set<EKind> operators = {
      EKind::Add,
      EKind::Subtract,
    };
    while (operators.count(g_current->m_eKind)) {
        auto pResult = std::make_shared<Unary>();
        pResult->m_eKind = g_current->m_eKind;
        SkipCurrent();
        pResult->m_pSub = ParseUnary();
        return pResult;
    }
    return ParseOperand();
}

std::shared_ptr<Expression> Parser::ParseOperand()
{
    std::shared_ptr<Expression> pResult = nullptr;
    switch (g_current->m_eKind) {
    case EKind::NullLiteral:   pResult = ParseNullLiteral();      break;
    case EKind::TrueLiteral:
    case EKind::FalseLiteral:  pResult = ParseBooleanLiteral();   break;
    case EKind::NumberLiteral: pResult = ParseNumberLiteral();    break;
    case EKind::StringLiteral: pResult = ParseStringLiteral();    break;
    case EKind::LeftBraket:    pResult = ParseListLiteral();      break;
    case EKind::LeftBrace:     pResult = ParseMapLiteral();       break;
    case EKind::Identifier:    pResult = ParseIdentifier();       break;
    case EKind::LeftParen:     pResult = ParseInnerExpression();  break;
    default:                  std::cout << "잘못된 식입니다.";       throw;
    }
    return ParsePostfix(pResult);
}

std::shared_ptr<Expression> Parser::ParseNullLiteral()
{
    SkipCurrent(EKind::NullLiteral);
    auto pResult = std::make_shared<NullLiteral>();
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseBooleanLiteral()
{
    auto pResult = std::make_shared<BooleanLiteral>();
    pResult->m_bValue = g_current->m_eKind == EKind::TrueLiteral;
    SkipCurrent();
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseNumberLiteral()
{
    auto pResult = std::make_shared<NumberLiteral>();
    pResult->m_dValue = std::stod(g_current->m_strName);
    SkipCurrent(EKind::NumberLiteral);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseStringLiteral()
{
    auto pResult = std::make_shared<StringLiteral>();
    pResult->m_strValue = g_current->m_strName;
    SkipCurrent(EKind::StringLiteral);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseListLiteral()
{
    auto pResult = std::make_shared<ArrayLiteral>();
    SkipCurrent(EKind::LeftBraket);
    if (g_current->m_eKind != EKind::RightBraket) {
        do
        {
            pResult->m_vecValue.push_back(ParseExpression());
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightBraket);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseMapLiteral()
{
    auto pResult = std::make_shared<MapLiteral>();
    SkipCurrent(EKind::LeftBrace);
    if (g_current->m_eKind != EKind::RightBrace) {
        do {
            auto name = g_current->m_strName;
            SkipCurrent(EKind::StringLiteral);
            SkipCurrent(EKind::Colon);
            auto value = ParseExpression();
            pResult->m_mapValue[name] = value;
        } while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightBrace);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseIdentifier()
{
    auto pResult = std::make_shared<GetVariable>();
    pResult->m_strName = g_current->m_strName;
    SkipCurrent(EKind::Identifier);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseInnerExpression()
{
    SkipCurrent(EKind::LeftParen);
    auto pResult = ParseExpression();
    SkipCurrent(EKind::RightParen);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParsePostfix(std::shared_ptr<Expression> _pSub)
{
    while (true) {
        switch (g_current->m_eKind) {
        case EKind::LeftParen:  _pSub = ParseCall(_pSub);    break;
        case EKind::LeftBraket: _pSub = ParseElement(_pSub); break;
        case EKind::ClassAccess:_pSub = ParseClassAccess(_pSub);       break;
        default: return _pSub;
        }
    }
}

std::shared_ptr<Expression> Parser::ParseCall(std::shared_ptr<Expression> _pSub)
{
    auto pResult = std::make_shared<Call>();
    pResult->m_pSub = _pSub;
    SkipCurrent(EKind::LeftParen);
    if (g_current->m_eKind != EKind::RightParen) {
        do pResult->m_vecArgument.push_back(ParseExpression());
        while (SkipCurrentIf(EKind::Comma));
    }
    SkipCurrent(EKind::RightParen);
    return pResult;
}

std::shared_ptr<Expression> Parser::ParseElement(std::shared_ptr<Expression> _pSub)
{
    auto pResult = std::make_shared<GetElement>();
    pResult->m_pSub = _pSub;
    SkipCurrent(EKind::LeftBraket);
    pResult->m_pIndex = ParseExpression();
    SkipCurrent(EKind::RightBraket);
    return pResult;
}

std::shared_ptr<Class> Parser::ParseClass()
{
    auto pResult = std::make_shared<Class>();

    SkipCurrent(EKind::Class);
    pResult->m_strName = g_current->m_strName;
    SkipCurrent(EKind::Identifier);
    SkipCurrent(EKind::LeftBrace);
    do {
        EMemberAccess access = EMemberAccess::Private;
        if (SkipCurrentIf(EKind::Private)) {
            access = EMemberAccess::Private;
        }
        else if (SkipCurrentIf(EKind::Public)) {
            access = EMemberAccess::Public;
        }
        else if (SkipCurrentIf(EKind::Protected)) {
            access = EMemberAccess::Protected;
        }
        pResult->m_vecVariable.push_back({ .m_pVariable = ParseVariable(), .m_eAccess = access });
    } while (g_current->m_eKind != EKind::RightBrace);
    SkipCurrent(EKind::RightBrace);
    SkipCurrent(EKind::Semicolon);

    return pResult;
}

std::shared_ptr<GetClassAccess> Parser::ParseClassAccess(std::shared_ptr<Expression> _pSub)
{
    auto pResult = std::make_shared<GetClassAccess>();
    pResult->m_pSub = _pSub;
    SkipCurrent(EKind::ClassAccess);   
    pResult->m_pMember = ParseExpression();
    return pResult;
}

void Parser::SkipCurrent()
{
    g_current++;
}

void Parser::SkipCurrent(EKind _eKind)
{
    if (g_current->m_eKind != _eKind) {
        throw;
    }
    g_current++;
}

bool Parser::SkipCurrentIf(EKind _eKind)
{
    if (g_current->m_eKind != _eKind) {
        return false;
    }
    g_current++;
    return true;
}
