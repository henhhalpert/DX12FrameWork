#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <Debug/DXDebugLayer.h>

#include <D3D/DXContext.h>

int main()
{
	// Init debug layer 
	DXDebugLayer::Get().Init();

	// Create Device 
	if (DXContext::Get().Init())
	{
		while (true)
		{
			auto* cmdList = DXContext::Get().InitCommandList();
			DXContext::Get().ExecuteCommandList();
		}
		DXContext::Get().ShutDown();
	}

	// Terminate debug layer 
	DXDebugLayer::Get().ShutDown();
	return 0;
} 