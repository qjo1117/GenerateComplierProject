#include "Scanner.h"

constexpr bool Scanner::IsCharType(char c, Scanner::ECharType type) noexcept
{
    switch (type) {
    case Scanner::ECharType::NumberLiteral: {
        return '0' <= c && c <= '9';
    }
    case Scanner::ECharType::StringLiteral: {
        return 32 <= c && c <= 126 && c != '\'' && c != '\"';
    }
    case Scanner::ECharType::IdentifierAndKeyword: {
        return '0' <= c && c <= '9' ||
            'a' <= c && c <= 'z' ||
            'A' <= c && c <= 'Z';
    }
    case Scanner::ECharType::OperatorAndPunctuator: {
        return 33 <= c && c <= 47 ||
            58 <= c && c <= 64 ||
            91 <= c && c <= 96 ||
            123 <= c && c <= 126;
    }
    default: {
        return false;
    }
    }
}

constexpr Scanner::ECharType Scanner::GetCharType(char _char) noexcept
{
    if (' ' == _char || '\t' == _char || '\r' == _char || '\n' == _char) {
        return Scanner::ECharType::WhiteSpace;
    }
    if ('0' <= _char && _char <= '9') {
        return Scanner::ECharType::NumberLiteral;
    }
    if (_char == '\'' || _char == '\"') {
        return Scanner::ECharType::StringLiteral;
    }
    if ('a' <= _char && _char <= 'z' || 'A' <= _char && _char <= 'Z') {
        return Scanner::ECharType::IdentifierAndKeyword;
    }
    if (33 <= _char && _char <= 47 && _char != '\'' ||
        58 <= _char && _char <= 64 ||
        91 <= _char && _char <= 96 ||
        123 <= _char && _char <= 126) {
        return Scanner::ECharType::OperatorAndPunctuator;
    }
    return Scanner::ECharType::Unknown;
}

std::vector<CodeToken> Scanner::Scan(std::string _sourceCode)
{
    std::vector<CodeToken> result;
    _sourceCode += '\0';
    m_info.iter = _sourceCode.begin();
    int32 row = 1, col = 0, baseCol = 0;
    CodeToken token;
    while (*m_info != '\0') {
        if (*m_info == '\n') {
            ++row;
            col = 1;
            baseCol = m_info.index;
        }
        else {
            col = (m_info.index - baseCol) + 1;
        }

        switch (GetCharType(*m_info)) {
        case Scanner::ECharType::WhiteSpace: {
            m_info++;
            continue;
        }
        case Scanner::ECharType::NumberLiteral: {
            token = ScanNumberLiteral();
            break;
        }
        case Scanner::ECharType::StringLiteral: {
            token = ScanStringLiteral();
            break;
        }
        case Scanner::ECharType::IdentifierAndKeyword: {
            token = ScanIdentifierAndKeyword();
            break;
        }
        case Scanner::ECharType::OperatorAndPunctuator: {
            token = ScanOperatorAndPunctuator();
            break;
        }
        default: {
            std::cout << *m_info << " 사용할 수 없는 문자입니다.\n" << "Row : " << row << " Col : " << col << '\n';
            exit(1);
        }
        }

        token.row = row;
        token.col = col;
        result.push_back(token);
    }
    result.push_back({ .kind = EKind::EndOfToken });
    return result;
}

CodeToken Scanner::ScanNumberLiteral()
{
    std::string str;
    while (IsCharType(*m_info, Scanner::ECharType::NumberLiteral)) {
        str += *m_info;
        m_info++;
    }
    if (*m_info == '.') {
        str += *m_info++;
        while (IsCharType(*m_info, Scanner::ECharType::NumberLiteral)) {
            str += *m_info++;
        }
    }
    return { .name = str, .kind = EKind::NumberLiteral };
}

CodeToken Scanner::ScanStringLiteral()
{
    std::string str;
    m_info++;
    while (IsCharType(*m_info, Scanner::ECharType::StringLiteral)) {
        str += *m_info;
        m_info++;
    }
    if (*m_info != '\'' && *m_info != '\"') {
        std::cout << "문자열의 종료 문자가 없습니다.";
        exit(1);
    }
    m_info++;
    return { .name = str, .kind = EKind::StringLiteral };
}

CodeToken Scanner::ScanIdentifierAndKeyword()
{
    std::string str;
    while (IsCharType(*m_info, Scanner::ECharType::IdentifierAndKeyword)) {
        str += *m_info;
        m_info++;
    }
    EKind kind = ToKind(str);
    if (kind == EKind::Unknown) {
        kind = EKind::Identifier;
    }
    return { .name = str, .kind = kind };
}

CodeToken Scanner::ScanOperatorAndPunctuator()
{
    std::string str;
    while (IsCharType(*m_info, Scanner::ECharType::OperatorAndPunctuator)) {
        str += *m_info;
        m_info++;
    }
    while (str.empty() == false && ToKind(str) == EKind::Unknown) {
        str.pop_back();
        m_info--;
    }
    if (str.empty()) {
        std::cout << *m_info << " 사용할 수 없는 문자입니다.";
        exit(1);
    }
    return { .name = str, .kind = ToKind(str) };
}

