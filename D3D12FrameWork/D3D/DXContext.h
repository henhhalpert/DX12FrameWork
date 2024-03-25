#pragma once
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

class DXContext
{
	public:
		bool Init();
		void ShutDown();

		void SignalAndWait();
		ID3D12GraphicsCommandList* InitCommandList();
		void ExecuteCommandList();

		inline ComPointer<ID3D12Device9>& GetDevice()
		{
			return m_device;
		}
		inline ComPointer<ID3D12CommandQueue>& GetCommandQueue()
		{
			return m_cmdQueue;
		}
	private:
		ComPointer<ID3D12Device9>					m_device;
		ComPointer<ID3D12CommandQueue>				m_cmdQueue;

		ComPointer<ID3D12GraphicsCommandList5>		m_cmdList;
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
