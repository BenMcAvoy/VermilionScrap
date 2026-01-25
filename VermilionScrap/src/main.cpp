// ReSharper disable CppClangTidyPerformanceNoIntToPtr

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <format>

#include <src/utils/sig.h>

#include <dxgi.h>

namespace {
	typedef HRESULT(__stdcall* DXGIPresent_T)(IDXGISwapChain* pSwapChain, UINT syncInterval, UINT flags);
	DXGIPresent_T OriginalPresent = nullptr;
	LONG_PTR OriginalWndProc = 0;

	auto UninjectEvent = INVALID_HANDLE_VALUE;
	std::atomic<bool> FrameInFlight = false;
	
	// Present hook
	HRESULT __stdcall HookedPresent(IDXGISwapChain* pSwapChain, const UINT syncInterval, const UINT flags) {
		FrameInFlight.store(true, std::memory_order_release);
		OutputDebugStringA("[VermilionScrap] DXGI Present called!\n");
		const auto hr = OriginalPresent(pSwapChain, syncInterval, flags);
		FrameInFlight.store(false, std::memory_order_release);
		return hr;
	}
	
	// WndProc hook
	LRESULT CALLBACK HookedWndProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
		if (uMsg == WM_KEYDOWN) {
			if (wParam == VK_END) {
				SetEvent(UninjectEvent);
			}
		}
		
		return CallWindowProcW(
			reinterpret_cast<WNDPROC>(OriginalWndProc),
			hWnd,
			uMsg,
			wParam,
			lParam
		);
	}
	
	void SetPresent(const DXGIPresent_T pNewPresent) {
		static Vermilion::SigScanner scanner("GameOverlayRenderer64.dll");
		static auto render = scanner.Scan("48 8B 05 ? ? ? ? 44 8B C5 8B D6");

		const std::int32_t displacement = *reinterpret_cast<std::int32_t*>(render + 3);
		auto nextInstr = reinterpret_cast<std::uint8_t*>(render + 7);
		const std::uintptr_t pPresent = reinterpret_cast<std::uintptr_t>(nextInstr + displacement);
		
		OriginalPresent = *reinterpret_cast<DXGIPresent_T*>(pPresent);
		*reinterpret_cast<DXGIPresent_T*>(pPresent) = pNewPresent;
	}
	
	void SetWndProc(const WNDPROC pNewWndProc) {
		static HWND hWnd = FindWindowA("CONTRAPTION_WINDOWS_CLASS", nullptr);
		OriginalWndProc = SetWindowLongPtrA(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(pNewWndProc));
	}
	
	void WaitForUninject() {
		if (UninjectEvent == INVALID_HANDLE_VALUE) {
			UninjectEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, "VermilionScrap_UninjectEvent");
		}
		
		WaitForSingleObject(UninjectEvent, INFINITE);
	}
	
	DWORD MainThread(const HMODULE hModule) {
		SetPresent(HookedPresent);
		SetWndProc(HookedWndProc);
		
		// Wait for uninject event (signalled by WndProc)
		WaitForUninject();
		
		// Unhook
		SetPresent(OriginalPresent);
		SetWndProc(reinterpret_cast<WNDPROC>(OriginalWndProc));
		
		// Sleep to allow the render thread to exit any in-flight calls into our hooks
		do {
			Sleep(100);
		} while (FrameInFlight.load(std::memory_order_acquire));
			
		FreeLibraryAndExitThread(static_cast<HMODULE>(hModule), 0);
	}
}

BOOL WINAPI DllMain(const HINSTANCE hDll, const DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hDll);
		CreateThread(
			nullptr,
			0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread),
			hDll,
			0,
			nullptr
		);
	}

	return TRUE;
}