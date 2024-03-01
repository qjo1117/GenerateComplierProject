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

    struct ScannerInfo
    {
    public:
        ScannerInfo& operator++(int32)
        {
            iter++; ++index;
            return *this;
        }
        ScannerInfo& operator--(int32)
        {
            iter--; --index;
            return *this;
        }
        char operator*()
        {
            return *iter;
        }

    public:
        std::string::iterator iter;
        int32 index = 0;
    };
private:
    Scanner() { }
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
    CodeToken ScanNumberLiteral();
    CodeToken ScanStringLiteral();
    CodeToken ScanIdentifierAndKeyword();
    CodeToken ScanOperatorAndPunctuator();

private:
    ScannerInfo m_info;
};

