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

	// Create Device 
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		while (!DXWindow::Get().ShouldClose())
		{
			DXWindow::Get().Update();
			ID3D12GraphicsCommandList* cmdList = DXContext::Get().InitCommandList();

			// Drawing shit on screen 

			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flushing all buffers 
		DXContext::Get().Flush(DXWindow::GetFrameCount());

		DXWindow::Get().ShutDown();
		DXContext::Get().ShutDown();
	}

	// Terminate debug layer 
	DXDebugLayer::Get().ShutDown();
	return 0;
} 