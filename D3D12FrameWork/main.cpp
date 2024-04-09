//#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/ImageLoader.h>
#include <Support/Window.h>
#include <Support/Shader.h>

#include <Debug/DXDebugLayer.h>
#include <D3D/DXContext.h>

void colorPuke(float* color)
{
	static int pukeState = 0;
	color[pukeState] += 0.01f;
	if (color[pukeState] > 1.0f)
	{
		pukeState++;
		if (pukeState == 3)
		{
			color[0] = 0.0f;
			color[1] = 0.0f;
			color[2] = 0.0f;
			pukeState = 0;
		}
	}
}

int main()
{
	// Init debug layer 
	DXDebugLayer::Get().Init();
	DXWindow& winInstance = DXWindow::Get();

	// Create Device 
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{

		// Heap properties for our Buffer Resource - D3D12_RESOURCE_DIMENSION_BUFFER - CPU SIDE
		D3D12_HEAP_PROPERTIES hpUpload{};
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD; // cpu side 
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpUpload.CreationNodeMask = 0; // multiple gpus no thx
		hpUpload.VisibleNodeMask = 0;
		// GPU SIDE
		D3D12_HEAP_PROPERTIES hpDefault{};
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT; // gpu side
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpDefault.CreationNodeMask = 0; // multiple gpus no thx
		hpDefault.VisibleNodeMask = 0;

		// Vertex Data 
		struct Vertex
		{
			float x, y;
			float u, v;
		};
		Vertex vertices[] =
		{
			// Triangle #1 
			{   -1.f, -1.f, 0.0f, 1.0f },
			{   0.f,   1.f, 0.5f, 0.0f },
			{   1.f,  -1.f, 1.0f, 1.0f }
		};

		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			// 2D Position layout. 
			// D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA -  once per vertex when processing vertices
			// if needed more than 1 object: can count the # of bytes of previous elems, OR simply use D3D12_APPEND_ALIGNED_ELEMENT to calculate AlignedByteOffset
			{"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		// Texture data
		ImageLoader::ImageData textureData;
		ImageLoader::LoadImageFromDisk("./auge_512_512_BGRA_32BPP.png", textureData);
		size_t textureStride = textureData.width * (size_t)((textureData.bpp + 7) / 8);
		size_t textureSize = textureData.height * textureStride;

		// upload buffer desc - no need in height and depth since its just a buffer 
		D3D12_RESOURCE_DESC rdv{};
		rdv.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdv.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdv.Width = 1024;
		rdv.Height = 1;
		rdv.DepthOrArraySize = 1;
		rdv.MipLevels = 1;
		rdv.Format = DXGI_FORMAT_UNKNOWN;
		rdv.SampleDesc.Count = 1;
		rdv.SampleDesc.Quality = 0;
		rdv.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdv.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_DESC rdu{};
		rdu.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdu.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdu.Width = textureSize + 1024;
		rdu.Height = 1;
		rdu.DepthOrArraySize = 1;
		rdu.MipLevels = 1;
		rdu.Format = DXGI_FORMAT_UNKNOWN;
		rdu.SampleDesc.Count = 1;
		rdu.SampleDesc.Quality = 0;
		rdu.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdu.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer;
		// created on cpu side for uploading to gpu
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rdu, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uploadBuffer));
		// create on gpu side 
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdv, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

		// Describing Texture upload 
		D3D12_RESOURCE_DESC rdt{};
		rdt.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rdt.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdt.Width = textureData.width;
		rdt.Height = textureData.height;
		rdt.DepthOrArraySize = 1;
		rdt.MipLevels = 1;
		rdt.Format = textureData.giPixelFormat;
		rdt.SampleDesc.Count = 1;
		rdt.SampleDesc.Quality = 0;
		rdt.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rdt.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPointer<ID3D12Resource2> texture;
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&texture));

		// == Descriptor heap for texture(s) ==
		D3D12_DESCRIPTOR_HEAP_DESC dhd{};
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		dhd.NumDescriptors = 8;
		dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		dhd.NodeMask = 0; // single gpu support 

		ComPointer<ID3D12DescriptorHeap> srvheap;
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&srvheap));

		// Create actual shader resource views - srv
		D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
		srv.Format = textureData.giPixelFormat;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv.Texture2D.MipLevels = 1;
		srv.Texture2D.MostDetailedMip = 0;
		srv.Texture2D.PlaneSlice = 0;
		srv.Texture2D.ResourceMinLODClamp = 0.0f;

		DXContext::Get().GetDevice()->CreateShaderResourceView(texture, &srv, srvheap->GetCPUDescriptorHandleForHeapStart());

		// Copy void* onto CPU Resource
		char* uploadBufferAddress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1024 + textureSize;
		uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAddress);
		memcpy(&uploadBufferAddress[0], textureData.data.data(), textureSize);
		memcpy(&uploadBufferAddress[textureSize], vertices, sizeof(vertices));
		uploadBuffer->Unmap(0, &uploadRange);
		// copy Cpu resource onto GPU resource (uploadBuffer --> vertexBuffer)
		ID3D12GraphicsCommandList6* preCmdList = DXContext::Get().InitCommandList();
		preCmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, textureSize, 1024);
		D3D12_BOX textureSizeAsBox;
		textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
		textureSizeAsBox.right = textureData.width;
		textureSizeAsBox.bottom = textureData.height;
		textureSizeAsBox.back = 1;
		D3D12_TEXTURE_COPY_LOCATION txtSource{}, txtDest{};
		txtSource.pResource = uploadBuffer;
		txtSource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		txtSource.PlacedFootprint.Offset = 0;
		txtSource.PlacedFootprint.Footprint.Width = textureData.width;
		txtSource.PlacedFootprint.Footprint.Height= textureData.height;
		txtSource.PlacedFootprint.Footprint.Depth = 1;
		txtSource.PlacedFootprint.Footprint.RowPitch = textureStride;
		txtSource.PlacedFootprint.Footprint.Format = textureData.giPixelFormat;
		txtDest.pResource = texture;
		txtDest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		txtDest.SubresourceIndex = 0;

		preCmdList->CopyTextureRegion(&txtDest, 0, 0, 0, &txtSource, &textureSizeAsBox);
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
		gfxPsod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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
		gfxPsod.NumRenderTargets = 1;
		gfxPsod.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gfxPsod.DSVFormat = DXGI_FORMAT_UNKNOWN;
		gfxPsod.BlendState.AlphaToCoverageEnable  = FALSE;
		gfxPsod.BlendState.IndependentBlendEnable = FALSE;
		
		gfxPsod.BlendState.RenderTarget[0].BlendEnable = FALSE;
		gfxPsod.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		// color components
		gfxPsod.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		// alpha components 
		gfxPsod.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].BlendOpAlpha= D3D12_BLEND_OP_ADD;
		gfxPsod.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		gfxPsod.BlendState.RenderTarget[0].RenderTargetWriteMask= D3D12_COLOR_WRITE_ENABLE_ALL;

		gfxPsod.DepthStencilState.DepthEnable = FALSE;
		gfxPsod.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPsod.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;


		gfxPsod.DepthStencilState.StencilEnable = FALSE;
		gfxPsod.DepthStencilState.StencilReadMask = 0;
		gfxPsod.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		gfxPsod.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPsod.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPsod.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

		gfxPsod.SampleMask = 0xFFFFFFFF;
		gfxPsod.SampleDesc.Count = 1;
		gfxPsod.SampleDesc.Quality = 0;

		gfxPsod.NodeMask = 0;
		gfxPsod.CachedPSO.CachedBlobSizeInBytes = 0;
		gfxPsod.CachedPSO.pCachedBlob = nullptr;
		gfxPsod.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


		ComPointer<ID3D12PipelineState> pso;
		DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(&pso));

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

			// == PSO ==
			cmdList->SetPipelineState(pso);
			cmdList->SetGraphicsRootSignature(rootSignature);
			cmdList->SetDescriptorHeaps(1, &srvheap);

			// == IA ==
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// == RS  ==
			
			//! Map normalized device coordinates(NDC), which range from -1 to 1 on each axis, to the actual screen coordinates. 
			//! ensures that rendered scene is displayed correctly to the user. 
			D3D12_VIEWPORT vp{};
			vp.TopLeftX = 0.f;
			vp.TopLeftY = 0.f;
			vp.Width    = (float)DXWindow::Get().GetWidth();
			vp.Height   = (float)DXWindow::Get().GetHeight();
			vp.MinDepth = 1.f;
			vp.MaxDepth = 0.f;
			cmdList->RSSetViewports(1, &vp);

			//! A scissor rectangle is a region of the render target (typically the screen) 
			//! where rendering is allowed to occur. Any pixels outside this rectangle are clipped and not drawn.
			RECT scRect;
			scRect.left = scRect.top = 0;
			scRect.right = DXWindow::Get().GetWidth();
			scRect.bottom = DXWindow::Get().GetHeight();
			cmdList->RSSetScissorRects(1, &scRect);

			// ROOT 
			static float color[] = { 0.0f, 0.0f, 0.0f };
			colorPuke(color);
			cmdList->SetGraphicsRoot32BitConstants(0, 3, color, 0);
			cmdList->SetGraphicsRootDescriptorTable(1, srvheap->GetGPUDescriptorHandleForHeapStart());

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