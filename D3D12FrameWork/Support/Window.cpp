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

    // Create Heap desc. for RTV
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
    //ZeroMemory(&descHeapDesc, sizeof(descHeapDesc));
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descHeapDesc.NumDescriptors = FrameCount; // 2 buffers, 2 frames. One desc. for each frame
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descHeapDesc.NodeMask = 0; // single gpu support 
    if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_rtvDescHeap))))
    {
        return false;
    }

    // Create handles that represent the location on the memory heap
    D3D12_CPU_DESCRIPTOR_HANDLE firstHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    UINT handleIncrement = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (size_t i = 0; i < FrameCount; ++i)
    {
        // initialize each handle to point to the beginning of the descriptor heap.
        m_rtvHandles[i] = firstHandle;
        // advancing the handle to point to the descriptor corresponding to the current frame, 
        // by adding the offset calculated based on the handle increment size multiplied by the frame index.
        m_rtvHandles[i].ptr += handleIncrement * i;
    }

    // Get Buffers
    if (!GetBuffers())
    {
        return false;
    }

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
    // Release d3d12 buffers resources
    ReleaseBuffers();

    // release rtv desc. heap 
    m_rtvDescHeap.Release();

    // Release swap chain 
    m_swapChain.Release();

    // Destroy and unregister window and window class respectively
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

void DXWindow::Resize()
{
    // treating "Swapchain cannot be resized unless all outstanding buffer references have been released" err
    // ie releasing d3d12 bufers prior to resizing buffers
    ReleaseBuffers();
    RECT clientRect;
    
    if (GetClientRect(m_window, &clientRect))
    {
        m_width = clientRect.right - clientRect.left;
        m_height = clientRect.bottom - clientRect.top;

        m_swapChain->ResizeBuffers(GetFrameCount(), m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
        m_shouldResize = false;
    }

    // Get d3d12 buffers again after resizing 
    GetBuffers();
}

void DXWindow::SetFullScreen(bool enabled)
{
    // Update Win styles
    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
    if (enabled)
    {
        // alter styles 
        style = WS_POPUP | WS_VISIBLE;
        exStyle = WS_EX_APPWINDOW;
    }

    // Apply new win style 
    SetWindowLongW(m_window, GWL_STYLE, style);
    SetWindowLongW(m_window, GWL_EXSTYLE, exStyle);

    if (enabled)
    {
        HMONITOR monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (GetMonitorInfoW(monitor, &monitorInfo))
        {
            // resize window to full screen
            SetWindowPos(m_window, nullptr,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                //! z order(the order of overlapping windows on the screen) should not be modified. 
                //! the last window created or brought to the front will have the highest Z order
                SWP_NOZORDER 
            );

            // Adjust buffers accordingly
            //Resize();
        }
    }

    else
    {
        ShowWindow(m_window, SW_MAXIMIZE);
    }

    m_isFullScreen = enabled;
}

void DXWindow::BeginFrame(ID3D12GraphicsCommandList6* cmdList)
{
    //! Change resource state to "draw"
    
    // Get current swap chain's back buffer, so we will know what buffer resource to work with.
    m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    // Resource transition 
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.Transition.pResource = m_buffers[m_currentBufferIndex];
    barr.Transition.Subresource = 0;
    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    cmdList->ResourceBarrier(1, &barr);

    float clearColor[] = { 0.8f, 0.5f, 0.5f, 0.5f };
    cmdList->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], clearColor, 0, nullptr );
    cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex] , false, nullptr);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList6* cmdList)
{
    //! Change resource state to "Present"

    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.Transition.pResource = m_buffers[m_currentBufferIndex];
    barr.Transition.Subresource = 0;
    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &barr);
}

bool DXWindow::GetBuffers()
{
    for (UINT i = 0; i < FrameCount; ++i)
    {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]))))
        {
            return false;
        }

        // Create a desc on the heap 
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;
        
        // Create a render target view (RTV) for a resource, such as a texture/buffer, in this case a buffer,
        // & bind it to a descriptor in the descriptor heap.
        DXContext::Get().GetDevice()->CreateRenderTargetView(m_buffers[i], &rtvDesc, m_rtvHandles[i]);
    }

    return true;
}

void DXWindow::ReleaseBuffers()
{
    for (size_t i = 0; i < FrameCount; ++i)
    {
        m_buffers[i].Release();
    }
}

LRESULT DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // FullScreen mode
    case WM_KEYDOWN:
        if (wParam == VK_F11)
        {
            Get().SetFullScreen(!Get().IsFullScreen());
        }
    case WM_SIZE:
        // Dynamically react to window resizing - edge cases - minimizing and maximizing screen - we do not want to resize in this case. 
        // ie lParam should not be 0 or lParam = width
        // WORD - lowest 16 bits
        // docs: The LOWORD and HIWORD macros get the 16-bit width and height values from lParam respectively.
        if (lParam && (LOWORD(lParam) != Get().m_width || HIWORD(lParam) != Get().m_height))
        {
            Get().m_shouldResize = true;
        }
        break;
    case WM_CLOSE:
        // Handle the close message
        Get().m_shouldClose = true;
        PostQuitMessage(2);
        return 0;
    }

    return DefWindowProcW(wnd, msg, wParam, lParam);
}
