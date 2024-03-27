#include "Window.h"

bool DXWindow::Init()
{
    // create window class
	WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = &DXWindow::OnWindowMessage;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName  = nullptr;
    wcex.lpszClassName = L"D3D12ExWndCls";
    wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

    // Register Class 
    m_wndClass = RegisterClassExW(&wcex);
    if (m_wndClass == 0)
    {
        return false;
    }

    // Get monitor details 
    POINT pos{ 0,0 };
    GetCursorPos(&pos);
    HMONITOR monitor = MonitorFromPoint(pos, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfoW(monitor, &monitorInfo);

    // Create a window 
    m_window = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
        LPCWSTR(m_wndClass),
        L"D3D12FrameWork",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        monitorInfo.rcWork.left + 100,
        monitorInfo.rcWork.top  + 100,
        1920,
        1080,
        nullptr,
        nullptr,
        wcex.hInstance,
        nullptr);

    if (m_window == nullptr)
    {
        return false;
    }
    // descriptors for swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFScreenDesc{};
    swapChainDesc.Width  = 1920;
    swapChainDesc.Height = 1080;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // unsigned normalized : 0 - 1 
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc.Count = 1; // 1 pixel per pixel 
    swapChainDesc.SampleDesc.Quality = 0; // no multisampling anti-aliasing
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = GetFrameCount(); // back buffer, front buffer and  sometimes middle buffer;  When the front buffer is done displaying and is ready to be swapped, it will take the contents of the buffer (either back or middle) that has the most recently completed frame. This way, the most up-to-date frame is always displayed, which can help reduce lag and tearing.
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    swapChainFScreenDesc.Windowed = true;
    
    // Create Swap Chain 
    ComPointer<IDXGIFactory7>& factory = DXContext::Get().GetFactory();
    ComPointer<IDXGISwapChain1> swapChain1;
    factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue(), m_window, &swapChainDesc, &swapChainFScreenDesc, nullptr, &swapChain1);
    if (!swapChain1.QueryInterface(m_swapChain))
        return false;
    
    return true;
}

void DXWindow::Update()
{
    MSG msg;
    //! retrieve messages from the message queue.
    //!retrieve (peek at) a message from the message queue associated with 
    //! the thread that created the specified 
    while (PeekMessageW(&msg, m_window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        //send the message retrieved by functions like GetMessage or PeekMessageW to the appropriate window procedure 
        DispatchMessageW(&msg);
    }
}

void DXWindow::ShutDown()
{
    m_swapChain.Release();
    if (m_window)
    {
        DestroyWindow(m_window);
    }
    if (m_wndClass)
    {
        UnregisterClassW((LPCWSTR)m_wndClass, GetModuleHandleW(nullptr));
    }
    
}

void DXWindow::Present()
{
    // 1st arg - num of frames to wait for sync; 2nd - swap chain flags 
    m_swapChain->Present(1, 0);
}

LRESULT DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        // Handle the close message
        Get().m_shouldClose = true;
        PostQuitMessage(2);
        return 0;
    case WM_SIZE:
        // Handle resizing
        // You may want to handle resizing here if you're using DirectX
        return 0;
    }

    return DefWindowProcW(wnd, msg, wParam, lParam);
}
