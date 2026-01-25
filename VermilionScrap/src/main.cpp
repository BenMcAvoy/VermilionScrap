// ReSharper disable CppClangTidyPerformanceNoIntToPtr

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "utils/overlay.h"

#include <imgui.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	auto UninjectEvent = INVALID_HANDLE_VALUE;
	auto ShowMenu = false;
	
	void Render(const Vermilion::RenderData& renderData) {
		// TODO: Watermark
		
		if (!ShowMenu)
			return;
		
		ImGui::Begin("Hello!");
		ImGui::Text("Press F10 to uninject.");
		ImGui::End();
	}
    
	HRESULT WndProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, LPARAM lParam) {
		if ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_F10) {
			SetEvent(UninjectEvent);
			return 1;
		}
		
		if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
			ShowMenu = !ShowMenu;
			return 1;
		}
		
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) || ShowMenu)
			return 1;

		return 0;
	}
    
	DWORD MainThread(const HMODULE hModule) {
		UninjectEvent = CreateEventA(
			nullptr,
			TRUE,
			FALSE,
			nullptr
		);
		
		Vermilion::HookOverlay(Render, WndProc);
		WaitForSingleObject(UninjectEvent, INFINITE);
		Vermilion::ShutdownOverlay();
			
		FreeLibraryAndExitThread(hModule, 0);
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