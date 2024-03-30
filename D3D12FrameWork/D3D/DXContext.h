#pragma once
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

class DXContext
{
	public:
		bool Init();
		void ShutDown();

		// Signaling and waiting for the GPU to finish its work ensures that all GPU operations are completed
		// before you proceed with shutting down the rendering process and releasing resources.
		// This is why you typically flush the command queueand wait for all operations to complete before closing the window.
		inline void Flush(size_t count)
		{
			for (size_t i = 0; i < count; ++i)
			{
				SignalAndWait();
			}
		}
		void SignalAndWait();
		ID3D12GraphicsCommandList6* InitCommandList();
		void ExecuteCommandList();

		inline ComPointer<ID3D12Device9>& GetDevice()
		{
			return m_device;
		}
		inline ComPointer<ID3D12CommandQueue>& GetCommandQueue()
		{
			return m_cmdQueue;
		}
		inline ComPointer<IDXGIFactory7>& GetFactory()
		{
			return m_dxgiFactory;
		}

	private:
		ComPointer<IDXGIFactory7> m_dxgiFactory;
		ComPointer<ID3D12Device9>					m_device;
		ComPointer<ID3D12CommandQueue>				m_cmdQueue;

		ComPointer<ID3D12GraphicsCommandList6>		m_cmdList;
		ComPointer<ID3D12CommandAllocator>			m_cmdAllocator;

		ComPointer<ID3D12Fence1>		m_fence;
		UINT64 m_fenceValue				= 0;
		HANDLE m_fenceEvent				= nullptr;
		// Singelton 
	public:
		static DXContext& Get()
		{
			static DXContext instance;
			return instance;
		}
	
		//! By disabling these, you prevent the possibility of creating copies of the singleton object,
		//! which could violate the singleton pattern by having multiple instances.
		DXContext(const DXContext&) = delete;
		DXContext& operator=(const DXContext&) = delete;
	
	private:
		DXContext() = default;
	
};
