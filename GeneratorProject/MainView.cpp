#include "MainView.h"

// Directory내 파일을 읽기 위한 친구
#include <filesystem>
#include <windows.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cstdlib>
namespace fs = std::filesystem;

#include "Token.h"
#include "Scanner.h"
#include "Parser.h"
#include "Object.h"

namespace
{
    // 문자열을 주어진 길이로 왼쪽으로 정렬하는 함수
    std::string LeftAlign(const std::string& _str, size_t _width) {
        if (_str.length() >= _width) {
            return _str;
        }
        return _str + std::string(_width - _str.length(), ' ');
    }

    // 문자열을 주어진 길이로 오른쪽으로 정렬하는 함수
    std::string RightAlign(const std::string& _str, size_t _width) {
        if (_str.length() >= _width) {
            return _str;
        }
        return std::string(_width - _str.length(), ' ') + _str;
    }

    std::string GetExecutablePath() {
        std::string path;
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        path = buffer;
        return path;
    }

    std::string PrintTokenListKind(std::vector<CodeToken> _tokenList)
    {
        std::string strResult = "KIND\n";
        for (auto& token : _tokenList) {
            strResult += ToKindString(token) + "\n";
        }
        return strResult;
    }

    std::string PrintTokenListString(std::vector<CodeToken> _tokenList)
    {
        std::string strResult = "STRING\n";
        for (auto& token : _tokenList) {
            strResult += ToTokenString(token) + "\n";
        }
        return strResult;
    }

    std::string PrintSyntaxTree(std::shared_ptr<Program> _pProgram)
    {
        string strResult;
        for (auto& pNode : _pProgram->m_vecClass) {
            strResult += pNode->PrintInfo(0);
        }
        for (auto& pNode : _pProgram->m_vecFunction) {
            strResult += pNode->PrintInfo(0);
        }
        return strResult;
    }

    std::string PrintObjectCode(std::tuple<CodeList, FunctionMap>& _tupleCodeTable)
    {
        string strResult = "FUNCTION\t\tADDESS\n";
        CodeList& vecCode = std::get<0>(_tupleCodeTable);
        FunctionMap& mapFunction = std::get<1>(_tupleCodeTable);

        strResult += string(18, '-') + '\n';
        for (auto& [strFuncName, funcAddress] : mapFunction) {
            strResult += std::format("{0}\t\t{1}\n", strFuncName, funcAddress);
        }
        strResult += '\n';
        
        strResult += "ADDR\tINSTRUCTION\tOPERAND\n";
        strResult += string(36, '-') + '\n';
        for (uint64 i = 0; i < vecCode.size(); ++i) {
            strResult += std::format("{0}\t{1}\n", RightAlign(std::to_string(i), 8), PrintCode(vecCode[i]));
        }
        return strResult;
    }

    // 컴파일러로 돌아가는 친구들 담은 전역변수 (클래스 구조까지는 굳이..?)
    std::map<std::string, std::wstring> g_mapPMFilePath;
    std::vector<std::string> g_vecRemoveFile;
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
}

MainView::MainView()
{
    m_strInputFileBuffer.reserve(Input_file_buffer_size);
}

MainView::~MainView()
{
}

void MainView::Init()
{
    ReadDirectory();
}

void MainView::Update()
{
    auto FuncPrintText = [](std::string& _str) {
        ImGui::BeginGroup();
        ImGui::Text("===================");
        ImGui::Text(_str.c_str());
        ImGui::Text("===================");
        ImGui::EndGroup();
        };

    if (ImGui::Button("Reload File Directory")) {
        ReadDirectory();
    }

    ImGui::SameLine();
    if (ImGui::Button("Create File")) {
        string fileName = m_strInputFileBuffer.data();
        if (fileName.find(".pm") == std::string::npos) {
            fileName += ".pm";
        }
        std::stringstream templateStringBuffer;
        std::ifstream templateFile(g_directory + "\\" + "pm_language_template.template");
        if (templateFile.is_open()) {
            templateStringBuffer << templateFile.rdbuf();
            templateFile.close();
        }

        std::ofstream file(g_directory + "\\" + fileName);
        if (file.is_open()) {
            file << templateStringBuffer.str();
            file.close();
            ReadDirectory();
        }
    }

    if (ImGui::Button("Complie")) {
        auto findIt = g_mapPMFilePath.find(m_strInputFileBuffer.data());
        if (findIt == g_mapPMFilePath.end()) {
            return;
        }

        std::ifstream file;
        file.open(g_directory + "\\" + findIt->first);
        if (file.is_open()) {
            std::stringstream stringBuffer;
            stringBuffer << file.rdbuf();

            m_strFileContext = stringBuffer.str();
            try
            {
                auto tokenList = Scanner::GetInstance().Scan(m_strFileContext);
                m_strPrintTokenKindText = PrintTokenListKind(tokenList);
                m_strPrintTokenStringText = PrintTokenListString(tokenList);

                m_pProgram = Parser::GetInstance().Parse(tokenList);
                m_strParserText = PrintSyntaxTree(m_pProgram);
            }
            catch(std::out_of_range& e)
            {
                file.close();
            }

            file.close();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Generate")) {
        auto findIt = g_mapPMFilePath.find(m_strInputFileBuffer.data());
        if (findIt == g_mapPMFilePath.end()) {
            return;
        }

        std::ifstream file;
        file.open(g_directory + "\\" + findIt->first);
        if (file.is_open()) {
            std::stringstream stringBuffer;
            stringBuffer << file.rdbuf();

            m_strFileContext = stringBuffer.str();
            try
            {
                auto tokenList = Scanner::GetInstance().Scan(m_strFileContext);
                m_strPrintTokenKindText = PrintTokenListKind(tokenList);
                m_strPrintTokenStringText = PrintTokenListString(tokenList);

                m_pProgram = Parser::GetInstance().Parse(tokenList);
                m_strParserText = PrintSyntaxTree(m_pProgram);
                m_codeTable = Generater::GetInstance().Generate(m_pProgram);
                m_strGenerateText = PrintObjectCode(m_codeTable);
            }
            catch (std::out_of_range& e)
            {
                file.close();
            }

            file.close();
        }
    }
    ImGui::SameLine();
    ImGui::InputText("File Name", m_strInputFileBuffer.data(), Input_file_buffer_size);

    if (ImGui::BeginListBox("File List", ImVec2{ 400, 200 })) {
        for (auto& [key, wStrFileName] : g_mapPMFilePath) {
            if (ImGui::Button(key.c_str(), ImVec2{ 250, 25 })) {
                m_strInputFileBuffer = key;
            }
            ImGui::SameLine();
            if (ImGui::Button(("FileOpen" + string("##") + key).c_str())) {
                string strOpenCommand = "code ..\\Output\\";
                strOpenCommand += key;
                system(strOpenCommand.c_str());
            }
            ImGui::SameLine();
            if (ImGui::Button(("Delete" + string("##") + key).c_str())) {
                g_vecRemoveFile.push_back(key);

                std::ifstream file(g_directory + "\\" + key);
                if(file.is_open()) {
                    std::stringstream stringBuffer;
                    stringBuffer << file.rdbuf();
                    m_strBackUp = stringBuffer.str();

                    std::ofstream backupFile(g_directory + "\\" + "backup.pm");
                    if (backupFile.is_open()) {
                        backupFile << m_strBackUp;
                        backupFile.close();
                    }
                    file.close();
                }
            }
        }
        ImGui::EndListBox();
    }

    if (m_strFileContext.empty() == false) {
        ImGui::SameLine();
        if (ImGui::BeginListBox("File Context", ImVec2{ 400, 200 })) {
            ImGui::Text(m_strFileContext.c_str());
            ImGui::EndListBox();
        }
    }
    
    if (m_strPrintTokenKindText.empty() == false) {
        FuncPrintText(m_strPrintTokenKindText);
    }
    if (m_strPrintTokenStringText.empty() == false) {
        if (m_strPrintTokenKindText.empty() == false) {
            ImGui::SameLine();
        }
        FuncPrintText(m_strPrintTokenStringText);
    }
    if (m_strParserText.empty() == false) {
        if (m_strPrintTokenStringText.empty() == false) {
            ImGui::SameLine();
        }
        FuncPrintText(m_strParserText);
    }
    if (m_strGenerateText.empty() == false) {
        if (m_strParserText.empty() == false) {
            ImGui::SameLine();
        }
        FuncPrintText(m_strGenerateText);
    }
}

void MainView::PostUpdate()
{
    if (g_vecRemoveFile.empty() == false) {
        for (auto& filePath : g_vecRemoveFile) {
            g_mapPMFilePath.erase(filePath);
            std::remove((g_directory + "\\" + filePath).c_str());
        }
        g_vecRemoveFile.clear();
    }
}

void MainView::End()
{
}

void MainView::Interpret()
{
    if (m_pProgram)
    {
        Interpreter::GetInstance().Interpret(m_pProgram);
    }
}
