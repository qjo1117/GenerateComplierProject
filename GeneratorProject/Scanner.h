#pragma once

#include <string>
#include <vector>
#include "Token.h"


class Scanner
{
public:
    enum class ECharType : __int32
    {
        Unknown,
        WhiteSpace,
        NumberLiteral,
        StringLiteral,
        IdentifierAndKeyword,
        OperatorAndPunctuator,
    };

public:
    static Scanner& GetInstance()
    {
        static Scanner instance;
        return instance;
    }

public:
    std::vector<CodeToken> Scan(std::string sourceCode);

private:
    constexpr bool IsCharType(char c, ECharType type) noexcept;
    constexpr ECharType GetCharType(char _char) noexcept;
    CodeToken ScanNumberLiteral(std::string::iterator& _iter);
    CodeToken ScanStringLiteral(std::string::iterator& _iter);
    CodeToken ScanIdentifierAndKeyword(std::string::iterator& _iter);
    CodeToken ScanOperatorAndPunctuator(std::string::iterator& _iter);
};

