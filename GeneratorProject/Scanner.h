#pragma once

#include <string>
#include <vector>
#include "Token.h"

enum class ECharType : __int32
{
    Unknown,
    WhiteSpace,
    NumberLiteral,
    StringLiteral,
    IdentifierAndKeyword,
    OperatorAndPunctuator,
};

std::vector<CodeToken> scan(std::string sourceCode);
constexpr bool IsCharType(char c, ECharType type) noexcept;
constexpr ECharType GetCharType(char _char) noexcept;
CodeToken ScanNumberLiteral(std::string::iterator& _iter);
CodeToken ScanStringLiteral(std::string::iterator& _iter);
CodeToken ScanIdentifierAndKeyword(std::string::iterator& _iter);
CodeToken ScanOperatorAndPunctuator(std::string::iterator& _iter);