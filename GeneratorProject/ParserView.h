#pragma once
#include "IWindowView.h"
class ParserView :
    public IWindowView
{
public:
	ParserView();
	virtual ~ParserView();

public:
	void Init() override;
	void Update() override;
	void PostUpdate() override;
	void End() override;

private:

};

