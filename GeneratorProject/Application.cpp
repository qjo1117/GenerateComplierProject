#include "Application.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "MainView.h"
REGISTER_WINDOW_VIEW(MainView);
#include "ResultConsoleView.h"
REGISTER_WINDOW_VIEW(ResultConsoleView);

#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND _hWnd,
    UINT _msg,
    WPARAM _wParam,
    LPARAM _lParam);

namespace
{
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
    {
        return Application::GetInstance().MsgProc(hWnd, msg, wParam, lParam);
    }
}

Application::Application()
{

}

Application::~Application()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(m_hMainWindow);
}

LRESULT Application::MsgProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(_hWnd, _msg, _wParam, _lParam))
        return true;

    switch (_msg) {
    case WM_SIZE:
        // Reset and resize swapchain
        break;
    case WM_SYSCOMMAND:
        if ((_wParam & 0xfff0) == SC_KEYMENU) 
            return 0;
        break;
    case WM_MOUSEMOVE:
        // cout << "Mouse " << LOWORD(lParam) << " " << HIWORD(lParam) << endl;
        break;
    case WM_LBUTTONUP:
        // cout << "WM_LBUTTONUP Left mouse button" << endl;
        break;
    case WM_RBUTTONUP:
        // cout << "WM_RBUTTONUP Right mouse button" << endl;
        break;
    case WM_KEYDOWN:
        // cout << "WM_KEYDOWN " << (int)wParam << endl;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(_hWnd, _msg, _wParam, _lParam);
}

bool Application::InitWindow(int32 _width, int32 _height)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"PmVirtualMachine",
                     NULL };

    if (RegisterClassEx(&wc) == false) {
        return false;
    }

    m_iScreenWidth = _width;
    m_iScreenHeight = _height;
    RECT wr = { 0, 0, m_iScreenWidth, m_iScreenHeight };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

    m_hMainWindow = CreateWindow(wc.lpszClassName, L"Pm Language Virtual Mechine Project",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL, NULL, wc.hInstance, NULL);

    if (m_hMainWindow == 0) {
        return false;
    }

    ShowWindow(m_hMainWindow, SW_SHOWDEFAULT);
    UpdateWindow(m_hMainWindow);
    return true;
}

bool Application::InitDirectX()
{
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_HARDWARE;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3 };
    D3D_FEATURE_LEVEL featureLevel;

    if (FAILED(D3D11CreateDevice(
        nullptr,    
        driver_type, 
        0,
        createDeviceFlags, 
        featureLevels,     
        ARRAYSIZE(featureLevels), 
        D3D11_SDK_VERSION,     
        device.GetAddressOf(), 
        &featureLevel,         
        context.GetAddressOf() 
    ))) {
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        return false;
    }

    UINT numQualityLevels;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4,
        &numQualityLevels);

    if (FAILED(device.As(&m_pDevice))) {
        return false;
    }

    if (FAILED(context.As(&m_pContext))) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_iScreenWidth;  
    sd.BufferDesc.Height = m_iScreenHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
    sd.BufferCount = 2;                                
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage =
        DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hMainWindow;    
    sd.Windowed = TRUE;                 
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (numQualityLevels > 0) {
        sd.SampleDesc.Count = 4; 
        sd.SampleDesc.Quality = numQualityLevels - 1;
    }
    else {
        sd.SampleDesc.Count = 1; 
        sd.SampleDesc.Quality = 0;
    }

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        0, // Default adapter
        driver_type,
        0, // No software device
        createDeviceFlags, featureLevels, 1, D3D11_SDK_VERSION, &sd,
        m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), &featureLevel,
        m_pContext.GetAddressOf()))) {
        return false;
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (backBuffer) {
        m_pDevice->CreateRenderTargetView(backBuffer.Get(), NULL,
            m_pRenderTargetView.GetAddressOf());
    }
    else {
        return false;
    }

    ZeroMemory(&m_descScreenViewport, sizeof(D3D11_VIEWPORT));
    m_descScreenViewport.TopLeftX = 0;
    m_descScreenViewport.TopLeftY = 0;
    m_descScreenViewport.Width = float32(m_iScreenWidth);
    m_descScreenViewport.Height = float32(m_iScreenHeight);
    m_descScreenViewport.MinDepth = 0.0f;
    m_descScreenViewport.MaxDepth = 1.0f; 

    m_pContext->RSSetViewports(1, &m_descScreenViewport);

    // Create a rasterizer state
    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true; 

    m_pDevice->CreateRasterizerState(&rastDesc, m_pRasterizerSate.GetAddressOf());

    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    depthStencilBufferDesc.Width = m_iScreenWidth;
    depthStencilBufferDesc.Height = m_iScreenHeight;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (numQualityLevels > 0) {
        depthStencilBufferDesc.SampleDesc.Count = 4; 
        depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
    }
    else {
        depthStencilBufferDesc.SampleDesc.Count = 1;
        depthStencilBufferDesc.SampleDesc.Quality = 0;
    }
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0;
    depthStencilBufferDesc.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(
        &depthStencilBufferDesc, 0, m_pDepthStencilBuffer.GetAddressOf()))) {
    }
    m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), 0, &m_pDepthStencilView);

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depthStencilDesc.DepthEnable = true; // false
    depthStencilDesc.DepthWriteMask =
        D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc =
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    m_pDevice->CreateDepthStencilState(&depthStencilDesc, m_pDepthStencilState.GetAddressOf());

    return true;
}

bool Application::InitGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float32(m_iScreenWidth), float32(m_iScreenHeight));
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
    ImGui::StyleColorsDark();

    if (ImGui_ImplDX11_Init(m_pDevice.Get(), m_pContext.Get()) == false) {
        return false;
    }

    if (ImGui_ImplWin32_Init(m_hMainWindow) == false) {
        return false;
    }

    for (auto& [strName, pView] : m_vecWindowView) {
        pView->Init();
    }

    return true;
}

bool Application::Init(int32 _width, int32 _height)
{
    if (InitWindow(_width, _height) == false) {
        return false;
    }
    else if (InitDirectX() == false) {
        return false;
    }
    else if (InitGUI() == false) {
        return false;
    }

    return true;
}

void Application::UpdateGUI()
{
    for (auto& [name, pView] : m_vecWindowView) {
        ImGui::Begin(name.c_str());
        pView->Update();
        pView->PostUpdate();
        ImGui::End();
    }
}

void Application::PostUpdateGUI()
{
}

void Application::Update(float32 _deltaTime)
{
}

void Application::Rendering()
{
    // 사실 Imgui쓸려고 하는거라 따로 하는게 없어도 되긴함

    m_pContext->RSSetViewports(1, &m_descScreenViewport);

    float32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0);

    m_pContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(),
        m_pDepthStencilView.Get());
    m_pContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0);
}

bool Application::Run()
{
    if (Init(1024, 780) == false) {
        return false;
    }
    
    MSG msg = { 0 };
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame(); 

            UpdateGUI();
            ImGui::Render();
            Update(ImGui::GetIO().DeltaTime); 
            PostUpdateGUI();

            Rendering();

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); 

            m_pSwapChain->Present(1, 0);
        }
    }

    return true;
}

void Application::RegisterWindowView(std::shared_ptr<IWindowView> _pView, std::string _strName)
{
    m_vecWindowView.insert(make_pair(_strName, _pView));
}

std::shared_ptr<IWindowView> Application::GetWindowView(std::string _strName) const
{
    auto findIt = m_vecWindowView.find(_strName);
    if (findIt != m_vecWindowView.end()) {
        return findIt->second;
    }
    return nullptr;
}
