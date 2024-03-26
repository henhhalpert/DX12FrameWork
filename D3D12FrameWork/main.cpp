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
			DXContext::Get().ExecuteCommandList();
		}
		DXContext::Get().ShutDown();
	}
	// Terminate debug layer 
	DXWindow::Get().ShutDown();
	DXDebugLayer::Get().ShutDown();
	return 0;
} 