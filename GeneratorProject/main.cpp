#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#include "Token.h"
#include "Scanner.h"


void printTokenList(std::vector<CodeToken> tokenList)
{
    std::cout << std::setw(12) << std::left << "KIND" << "STRING" << std::endl;
    std::cout << std::string(23, '-') << std::endl;
    for (auto& token : tokenList) {
        std::cout << token << std::endl;
    }
}

int main()
{
    std::ifstream file;
    file.open("file_system.pm");
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();

        auto tokenList = Scanner::GetInstance().Scan(buffer.str());
        printTokenList(move(tokenList));
    }

    return 0;
}

