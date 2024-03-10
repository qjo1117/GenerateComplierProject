#pragma once
#include "IWindowView.h"
#include "Application.h"

class ResultConsoleView :
    public IWindowView
{
public:
	ResultConsoleView();
	virtual ~ResultConsoleView();

public:
	void Init() override;
	void Update() override;
	void PostUpdate() override;
	void End() override;

private:
	
};

