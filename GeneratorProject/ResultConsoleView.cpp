#include "ResultConsoleView.h"
#include "MainView.h"

ResultConsoleView::ResultConsoleView()
{
}

ResultConsoleView::~ResultConsoleView()
{
}

void ResultConsoleView::Init()
{
}

void ResultConsoleView::Update()
{
}

void ResultConsoleView::PostUpdate()
{
	auto pMainView = std::dynamic_pointer_cast<MainView>(Application::GetInstance().GetWindowView("MainView"));
	if (pMainView == nullptr) {
		return;
	}
	pMainView->Interpret();
}

void ResultConsoleView::End()
{
}
