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