#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

// Directory내 파일을 읽기 위한 친구
#include <filesystem>
#include <windows.h>
namespace fs = std::filesystem;

#include "Token.h"
#include "Scanner.h"
#include "Parser.h"

std::string GetExecutablePath() {
    std::string path;
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    path = buffer;
    return path;
}

void PrintTokenList(std::vector<CodeToken> tokenList)
{
    std::cout << std::setw(12) << std::left << "KIND" << "STRING" << std::endl;
    std::cout << std::string(23, '-') << std::endl;
    for (auto& token : tokenList) {
        std::cout << token << std::endl;
    }
}

void PrintSyntaxTree(std::shared_ptr<Program> _pProgram) 
{
    for (auto& node : _pProgram->m_vecFunction) {
        node->PrintInfo(0);
    }
}

// 컴파일러로 돌아가는 친구들 담은 전역변수 (클래스 구조까지는 굳이..?)
std::map<std::string, std::wstring> g_mapPMFilePath;
std::string g_directory;

// Directory내 파일을 읽어서 파일을 등록한다.
void ReadDirectory()
{  
    g_directory = fs::canonical(GetExecutablePath()).parent_path().string();
    auto executablePath = fs::canonical(GetExecutablePath()).parent_path();
    std::string directory = (executablePath).string();

    for (auto& file : fs::directory_iterator(directory)) {
        if (file.path().extension().compare(".pm") == 0) {
            auto fileName = file.path().filename().string();
            g_mapPMFilePath.insert(make_pair(fileName, file.path().filename().wstring()));
        }
    }
}

int main()
{
    ReadDirectory();
    std::cout << "=== 저장된 파일들 ===\n";
    for (auto& [key, wStrFileName] : g_mapPMFilePath) {
        std::cout << key << '\n';
    }
    std::cout << "=====================\n";

    while (true) {
        std::string strInput;
        std::cout << "컴파일 하고 싶은 파일을 입력하세요 : ";
        std::cin >> strInput;

        if (strInput.compare("print") == 0) {
            std::cout << "=== 저장된 파일들 ===\n";
            for (auto& [key, wStrFileName] : g_mapPMFilePath) {
                std::cout << key << '\n';
            }
            std::cout << "=====================\n";
        }
        else {

            auto findIt = g_mapPMFilePath.find(strInput);
            if (findIt != g_mapPMFilePath.end()) {
                std::ifstream file;
                file.open(g_directory + "\\" + findIt->first);
                if (file.is_open()) {
                    std::cout << "=============================================\n";
                    std::stringstream buffer;
                    buffer << file.rdbuf();

                    auto tokenList = Scanner::GetInstance().Scan(buffer.str());
                    PrintTokenList(tokenList);
                    std::cout << '\n' << '\n';
                    auto syntaxTree = Parser::GetInstance().Parse(tokenList);
                    PrintSyntaxTree(syntaxTree);
                    std::cout << "=============================================\n";
                    file.close();
                }
            }
            else {
                std::cout << "찾지 못했습니다." << '\n';
            }
        }
    }

    return 0;
}

