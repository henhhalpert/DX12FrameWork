//#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Window.h>
#include <Support/Shader.h>

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

		// Vertex Data 
		struct Vertex
		{
			float x, y;
		};
		Vertex vertices[] =
		{
			// Triangle #1 
			{  -1.f, -1.f},
			{   0.f,  1.f},
			{   1.f,  1.f}
		};

		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			// 2D Position layout. 
			// D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA -  once per vertex when processing vertices
			// if needed more than 1 object: can count the # of bytes of previous elems, OR simply use D3D12_APPEND_ALIGNED_ELEMENT to calculate AlignedByteOffset
			{"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

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
		memcpy(uploadBufferAddress, vertices, sizeof(vertices));
		uploadBuffer->Unmap(0, &uploadRange);

		// copy Cpu resource onto GPU resource (uploadBuffer --> vertexBuffer)
		ID3D12GraphicsCommandList6* preCmdList = DXContext::Get().InitCommandList();
		preCmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
		DXContext::Get().ExecuteCommandList();

		// === Shaders ===
		Shader rootSignatureShader("RootSignature.cso");
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		// Create root signature
		ComPointer<ID3D12RootSignature> rootSignature;
		HRESULT hr = DXContext::Get().GetDevice()->CreateRootSignature(0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));

		// Error creating root signature
		if (FAILED(hr))
			return -1;

		// Pipeline State
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
		gfxPsod.pRootSignature = rootSignature;
		gfxPsod.InputLayout.NumElements = _countof(vertexLayout);
		gfxPsod.InputLayout.pInputElementDescs = vertexLayout;
		gfxPsod.IBStripCutValue    = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		gfxPsod.VS.BytecodeLength  = vertexShader.GetSize();
		gfxPsod.VS.pShaderBytecode = vertexShader.GetBuffer();
		gfxPsod.PS.BytecodeLength  = pixelShader.GetSize();
		gfxPsod.PS.pShaderBytecode = pixelShader.GetBuffer();
		gfxPsod.DS.BytecodeLength  =  0;
		gfxPsod.DS.pShaderBytecode =  nullptr;
		gfxPsod.HS.BytecodeLength  =  0;
		gfxPsod.HS.pShaderBytecode =  nullptr;
		gfxPsod.GS.BytecodeLength  =  0;
		gfxPsod.GS.pShaderBytecode =  nullptr;
		gfxPsod.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // the interior should be filled with solid color.
		gfxPsod.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		gfxPsod.RasterizerState.FrontCounterClockwise = FALSE;
		gfxPsod.RasterizerState.DepthBias = 0;
		gfxPsod.RasterizerState.DepthBiasClamp = 0.f;
		gfxPsod.RasterizerState.SlopeScaledDepthBias = 0.f;
		gfxPsod.RasterizerState.DepthClipEnable = FALSE;
		gfxPsod.RasterizerState.MultisampleEnable = FALSE;
		gfxPsod.RasterizerState.AntialiasedLineEnable = FALSE;
		gfxPsod.RasterizerState.ForcedSampleCount = 0;
		gfxPsod.StreamOutput.NumEntries = 0;
		gfxPsod.StreamOutput.NumStrides = 0;
		gfxPsod.StreamOutput.pBufferStrides = nullptr;
		gfxPsod.StreamOutput.pSODeclaration = nullptr;
		gfxPsod.StreamOutput.RasterizedStream = 0;
		gfxPsod.NodeMask = 0;
		gfxPsod.CachedPSO.CachedBlobSizeInBytes = 0;
		gfxPsod.CachedPSO.pCachedBlob = nullptr;
		gfxPsod.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		// Vertex Buffer View
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(Vertex) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertex);

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

			// Input Assembler 
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Draw 
			cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);
			// Switch to 'Present' state
			DXWindow::Get().EndFrame(cmdList);

			// Finish Drawing and present 
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flush all GPU operations Upon Closure
		DXContext::Get().Flush(DXWindow::GetFrameCount());

		// Close resources 
		vertexBuffer.Release();
		uploadBuffer.Release();

		DXWindow::Get().ShutDown();
		DXContext::Get().ShutDown();
	}
	
	// Terminate debug layer 
	DXDebugLayer::Get().ShutDown();
	return 0;
} 