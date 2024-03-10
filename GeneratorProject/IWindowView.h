#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "TypeDefine.h"

#define REGISTER_WINDOW_VIEW_NAME(ViewType, ViewName)													\
struct Resgister_Dummy_##ViewType																	\
{																									\
public:																								\
	Resgister_Dummy_##ViewType()																	\
	{																								\
		Application::GetInstance().RegisterWindowView(std::make_shared<##ViewType>(), ViewName);	\
	}																								\
}Dummy_##ViewType													

#define REGISTER_WINDOW_VIEW(ViewType) REGISTER_WINDOW_VIEW_NAME(ViewType, #ViewType)


class IWindowView
{
public:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void PostUpdate() = 0;
	virtual void End() = 0;
};

