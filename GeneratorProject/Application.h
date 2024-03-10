#pragma once

#include "TypeDefine.h"
#include <windows.h>
#include <wrl.h>
#include <memory>

using Microsoft::WRL::ComPtr;

#include "Token.h"
#include "Scanner.h"
#include "Parser.h"
#include "Object.h"

#include "IWindowView.h"

#define USE_APPLICATION_IMGUI

class Application
{
private:
	Application();
	~Application();
public:
	static Application& GetInstance()
	{
		static Application instance;
		return instance;
	}

	static LRESULT MsgProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);
public:
	bool InitWindow(int32 _width, int32 _height);
	bool InitDirectX();
	bool InitGUI();
	bool Init(int32 _width, int32 _height);

	void UpdateGUI();
	void PostUpdateGUI();
	void Update(float32 _deltaTime);

	void Rendering();

	bool Run();
public:
	void RegisterWindowView(std::shared_ptr<IWindowView> _pView, std::string _strName);
	std::shared_ptr<IWindowView> GetWindowView(std::string _strName) const;

	// Print
	void Print(std::string _str) { m_strPrint += _str; }
	void PrintClear() { m_strPrint.clear(); }
	const std::string& GetPrint() const noexcept { return m_strPrint; }

private:
	int32 m_iScreenWidth = 0;
	int32 m_iScreenHeight = 0;
	HWND m_hMainWindow;

	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	ComPtr<IDXGISwapChain> m_pSwapChain;
	ComPtr<ID3D11RasterizerState> m_pRasterizerSate;

	// Depth buffer ฐüทร
	ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;

	D3D11_VIEWPORT m_descScreenViewport;

	std::map<std::string, std::shared_ptr<IWindowView>> m_vecWindowView;

	std::string m_strPrint;
};

