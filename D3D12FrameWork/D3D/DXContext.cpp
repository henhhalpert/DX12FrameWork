#include "DXContext.h"

bool DXContext::Init()
{
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory))))
    {
        return false;
    }

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device))))
    {
        return false;
    }

    // Command Queue Creation 
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0; // // Which gpu to target
    if (FAILED(m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_cmdQueue))))
    {
        return false;
    }

    // Create a fence
    if (FAILED(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
    {
        return false;
    }

    // Create a fence event for cpu-gpu sync. via fences
    m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    if (!m_fenceEvent)
    {
        return false;
    }

    // Create a command allocator 
    // Queue also for direct command, so make command allocator also of type direct
    if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator))))
        return false;

    // Frontend
    if (FAILED(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_cmdList))))
    {
        return false;
    }

    return true;
}

void DXContext::ShutDown()
{
    m_cmdList.Release();
    m_cmdAllocator.Release();
    if (m_fenceEvent)
        CloseHandle(m_fenceEvent);
    m_fence.Release();
    m_cmdQueue.Release();
    m_dxgiFactory.Release();
    m_device.Release();
}

void DXContext::SignalAndWait()
{
    m_cmdQueue->Signal(m_fence, ++m_fenceValue);
    //! Telling DirectX 12 to signal the Win32 event represented by m_fenceEvent when the fence reaches the value m_fenceValue
    //! Upon completion of the task, ie when m_fence reaches m_fenceValue 
    //! Once the fence value reaches or surpasses m_fenceValue, 
    //! indicating that the GPU has completed the associated operations, the event will be signaled.

    //! efficiently synchronize CPU and GPU operations without busy-waiting:
    if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
    {
        // wait for event, max 20 secs
        // WAIT_OBJECT_0 is used to indicate that the object you’re waiting for is ready (or “signaled”)
        if (WaitForSingleObject(m_fenceEvent, DWORD(2e4)) != WAIT_OBJECT_0)
        {
            CloseHandle(m_fenceEvent);
            std::exit(-1);
        }
    }
    else
    {
        std::exit(-1);
    }


    //Bad Approach - THIS IS BUSY WAITING (or polling) - VERY EXPENSIVE ! 
    // Instead of busy-waiting, it's recommended to use synchronization primitives like events, semaphores, 
    // or condition variables to efficiently synchronize GPU and CPU operations
    //while (m_fence->GetCompletedValue() < m_fenceValue)
    //{
    //    SwitchToThread();
    //}
}

ID3D12GraphicsCommandList6* DXContext::InitCommandList()
{
    m_cmdAllocator->Reset();
    m_cmdList->Reset(m_cmdAllocator, nullptr);
    return m_cmdList;
}

void DXContext::ExecuteCommandList()
{
    // close the list before executing it
    if (SUCCEEDED(m_cmdList->Close()))
    {
        UINT numCmdLists = 1;
        ID3D12CommandList* lists[] = { m_cmdList };
        m_cmdQueue->ExecuteCommandLists(numCmdLists, lists);
        // Sync using fences and wait for single event to occur.
        SignalAndWait();
    }
}
