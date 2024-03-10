#pragma once
#include "IWindowView.h"
#include "Application.h"

class MainView :
    public IWindowView
{
public:
	MainView();
	virtual ~MainView();

public:
	void Init() override;
	void Update() override;
	void PostUpdate() override;
	void End() override;

public:
	void Interpret();

private:
	const int64 Input_file_buffer_size = 1024;
	std::string m_strInputFileBuffer;

	std::string m_strFileContext;
	std::string m_strPrintTokenKindText;
	std::string m_strPrintTokenStringText;
	std::string m_strParserText;

	std::string m_strBackUp;

	std::shared_ptr<Program> m_pProgram = nullptr;
};

