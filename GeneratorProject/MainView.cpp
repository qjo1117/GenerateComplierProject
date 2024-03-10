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
        for (auto& node : _pProgram->m_vecFunction) {
            strResult += node->PrintInfo(0);
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
        ImGui::Text("==========================");
        ImGui::Text(_str.c_str());
        ImGui::Text("==========================");
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
            auto tokenList = Scanner::GetInstance().Scan(m_strFileContext);
            m_strPrintTokenKindText = PrintTokenListKind(tokenList);
            m_strPrintTokenStringText = PrintTokenListString(tokenList);

            m_pProgram = Parser::GetInstance().Parse(tokenList);
            m_strParserText = PrintSyntaxTree(m_pProgram);
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
    if (m_strParserText.empty() == false)
    {
        if (m_strPrintTokenStringText.empty() == false) {
            ImGui::SameLine();
        }
        FuncPrintText(m_strParserText);
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
