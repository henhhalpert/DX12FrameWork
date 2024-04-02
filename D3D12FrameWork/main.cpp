#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Window.h>

#include <Debug/DXDebugLayer.h>

#include <D3D/DXContext.h>

int main()
{
	// Init debug layer 
	DXDebugLayer::Get().Init();
	DXWindow& winInstance = DXWindow::Get();

	// Create Device 
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{

		// Putting data on gpu 
		const char* hello = "hello GPU!";

		D3D12_HEAP_PROPERTIES hpUpload{};
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD; // upload data from cpu to gpu
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpUpload.CreationNodeMask = 0; // multiple gpus no thx
		hpUpload.VisibleNodeMask = 0; 

		D3D12_HEAP_PROPERTIES hpDefault{};
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT; // upload data from cpu to gpu
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpDefault.CreationNodeMask = 0; // multiple gpus no thx
		hpDefault.VisibleNodeMask = 0;

		// upload buffer desc - no need in height and depth since its just a buffer 
		D3D12_RESOURCE_DESC rd{};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rd.Width = 1024;
		rd.Height = 1;
		rd.DepthOrArraySize = 1;
		rd.MipLevels = 1; // how often we want to create a resource. no mip mapping = 1
		rd.Format = DXGI_FORMAT_UNKNOWN;
		rd.SampleDesc.Count = 1; // not using it --> set to 1 
		rd.SampleDesc.Quality = 0;
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // makes sense for a buffer
		rd.Flags = D3D12_RESOURCE_FLAG_NONE; 

		ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer; 
		// created on cpu side for uploading to gpu
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uploadBuffer));
		// create on gpu side 
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

		// Copy void* onto CPU Resource 
		void* uploadBufferAddress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1023;

		// populate uploadBufferAddress for the CPU to use this address to access the mapped memory region.
		uploadBuffer->Map(0, &uploadRange, &uploadBufferAddress);
		// populate allocated memory with hello as an example, will be deleted later. 
		memcpy(uploadBufferAddress, hello, strlen(hello) + 1);
		uploadBuffer->Unmap(0, &uploadRange);

		// copy Cpu resource onto GPU resource (uploadBuffer --> vertexBuffer)
		ID3D12GraphicsCommandList6* preCmdList = DXContext::Get().InitCommandList();
		preCmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
		DXWindow::Get().SetFullScreen(true);
		while (!DXWindow::Get().ShouldClose())
		{
			// Process pending resource message
			DXWindow::Get().Update();

			// Handle Resizing 
			if (DXWindow::Get().ShouldResize())
			{
				DXContext::Get().Flush(DXWindow::GetFrameCount());
				winInstance.Resize();
			}
			// Begin Drawing
			ID3D12GraphicsCommandList6* cmdList = DXContext::Get().InitCommandList();

			// Switch to 'Draw' state
			DXWindow::Get().BeginFrame(cmdList);
			// Switch to 'Present' state
			DXWindow::Get().EndFrame(cmdList);
			
			// Finish Drawing and present 
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flush all GPU operations Upon Closure
		DXContext::Get().Flush(DXWindow::GetFrameCount());
		DXWindow::Get().ShutDown();
		DXContext::Get().ShutDown();
	}
	
	// Terminate debug layer 
	DXDebugLayer::Get().ShutDown();
	return 0;
} 