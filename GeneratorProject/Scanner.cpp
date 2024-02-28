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
    std::string::iterator current = _sourceCode.begin();
    while (*current != '\0') {
        switch (GetCharType(*current)) {
        case Scanner::ECharType::WhiteSpace: {
            current += 1;
            break;
        }
        case Scanner::ECharType::NumberLiteral: {
            result.push_back(ScanNumberLiteral(current));
            break;
        }
        case Scanner::ECharType::StringLiteral: {
            result.push_back(ScanStringLiteral(current));
            break;
        }
        case Scanner::ECharType::IdentifierAndKeyword: {
            result.push_back(ScanIdentifierAndKeyword(current));
            break;
        }
        case Scanner::ECharType::OperatorAndPunctuator: {
            result.push_back(ScanOperatorAndPunctuator(current));
            break;
        }
        default: {
            std::cout << *current << " 사용할 수 없는 문자입니다.";
            exit(1);
        }
        }
    }
    result.push_back({ EKind::EndOfToken });
    return result;
}

CodeToken Scanner::ScanNumberLiteral(std::string::iterator& _iter)
{
    std::string str;
    while (IsCharType(*_iter, Scanner::ECharType::NumberLiteral))
        str += *_iter++;
    if (*_iter == '.') {
        str += *_iter++;
        while (IsCharType(*_iter, Scanner::ECharType::NumberLiteral)) {
            str += *_iter++;
        }
    }
    return { EKind::NumberLiteral, str };
}

CodeToken Scanner::ScanStringLiteral(std::string::iterator& _iter)
{
    std::string str;
    _iter++;
    while (IsCharType(*_iter, Scanner::ECharType::StringLiteral))
        str += *_iter++;
    if (*_iter != '\'' && *_iter != '\"') {
        std::cout << "문자열의 종료 문자가 없습니다.";
        exit(1);
    }
    _iter++;
    return { EKind::StringLiteral, str };
}

CodeToken Scanner::ScanIdentifierAndKeyword(std::string::iterator& _iter)
{
    std::string str;
    while (IsCharType(*_iter, Scanner::ECharType::IdentifierAndKeyword)) {
        str += *_iter++;
    }
    EKind kind = ToKind(str);
    if (kind == EKind::Unknown)
        kind = EKind::Identifier;
    return { kind, str };
}

CodeToken Scanner::ScanOperatorAndPunctuator(std::string::iterator& _iter)
{
    std::string str;
    while (IsCharType(*_iter, Scanner::ECharType::OperatorAndPunctuator))
        str += *_iter++;
    while (str.empty() == false && ToKind(str) == EKind::Unknown) {
        str.pop_back();
        _iter--;
    }
    if (str.empty()) {
        std::cout << *_iter << " 사용할 수 없는 문자입니다.";
        exit(1);
    }
    return { ToKind(str), str };
}

