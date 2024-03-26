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

	return m_window != nullptr;
}

void DXWindow::Update()
{
    MSG msg;
    while (PeekMessageW(&msg, m_window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void DXWindow::ShutDown()
{
    if (m_window)
    {
        DestroyWindow(m_window);
    }
    if (m_wndClass != 0)
    {
        UnregisterClassW((LPCWSTR)m_wndClass, GetModuleHandleW(nullptr));
    }
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
