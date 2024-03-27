#pragma once

#pragma once
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <D3D/DXContext.h>

class DXWindow
{
	public:
		bool Init();
		void Update();
		void ShutDown();
		void Present();

		inline bool ShouldClose() const
		{
			return m_shouldClose;
		}
		static constexpr size_t GetFrameCount()
		{
			return 2;
		}

	private:

		// Singelton 
	public:
		static DXWindow& Get()
		{
			static DXWindow instance;
			return instance;
		}
	private:
		static LRESULT CALLBACK  OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
		ATOM m_wndClass = 0;
		HWND m_window = nullptr;
		bool m_shouldClose = false;

		ComPointer<IDXGISwapChain4> m_swapChain;

	public:
		//! By disabling these, you prevent the possibility of creating copies of the singleton object,
		//! which could violate the singleton pattern by having multiple instances.
		DXWindow(const DXWindow&) = delete;
		DXWindow& operator=(const DXWindow&) = delete;

	private:
		DXWindow() = default;
};
