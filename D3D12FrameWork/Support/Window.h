#pragma once

#pragma once
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

class DXWindow
{
	public:
		bool Init();
		void Update();
		void ShutDown();

		inline bool ShouldClose() const
		{
			return m_shouldClose;
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

	public:
		//! By disabling these, you prevent the possibility of creating copies of the singleton object,
		//! which could violate the singleton pattern by having multiple instances.
		DXWindow(const DXWindow&) = delete;
		DXWindow& operator=(const DXWindow&) = delete;

	private:
		DXWindow() = default;
};
