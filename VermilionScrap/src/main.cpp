// ReSharper disable CppClangTidyPerformanceNoIntToPtr

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "utils/overlay.h"

#include <imgui.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	auto UninjectEvent = INVALID_HANDLE_VALUE;
	auto ShowMenu = false;
	
	void Watermark() {
		ImDrawList* draw = ImGui::GetForegroundDrawList();
		const ImGuiIO& io = ImGui::GetIO();

		const auto title = "Vermilion";
		const auto sep   = "|";

		char fpsBuf[32];
		(void)snprintf(fpsBuf, sizeof(fpsBuf), "%.0f FPS", io.Framerate);

		constexpr ImVec2 padding(8.0f, 6.0f);
		constexpr float accentWidth = 3.0f;

		ImVec2 titleSize = ImGui::CalcTextSize(title);
		ImVec2 sepSize   = ImGui::CalcTextSize(sep);
		ImVec2 fpsSize   = ImGui::CalcTextSize(fpsBuf);

		const float textHeight = std::max({
			titleSize.y,
			sepSize.y,
			fpsSize.y
		});

		const float height =
			padding.y * 2.0f +
			textHeight;

		const float width =
			accentWidth +
			padding.x +
			titleSize.x +
			padding.x * 0.5f +
			sepSize.x +
			padding.x * 0.5f +
			fpsSize.x +
			padding.x;

		constexpr ImVec2 pos(10.0f, 10.0f);
		const ImVec2 end(pos.x + width, pos.y + height);

		constexpr ImU32 bgCol     = IM_COL32(20, 20, 20, 220);
		constexpr ImU32 textCol   = IM_COL32(230, 230, 230, 255);
		constexpr ImU32 fpsCol    = IM_COL32(160, 160, 160, 255);
		const ImU32 accentCol = ImColor{ 227, 66, 52 };

		draw->AddRectFilled(pos, end, bgCol, 4.0f);

		draw->AddRectFilled(
			pos,
			ImVec2(pos.x + accentWidth, end.y),
			accentCol,
			4.0f,
			ImDrawFlags_RoundCornersLeft
		);

		const float textY = pos.y + (height - textHeight) * 0.5f;
		float x = pos.x + accentWidth + padding.x;

		draw->AddText(ImVec2(x, textY), accentCol, title);
		x += titleSize.x + padding.x * 0.5f;

		draw->AddText(ImVec2(x, textY), textCol, sep);
		x += sepSize.x + padding.x * 0.5f;

		draw->AddText(ImVec2(x, textY), fpsCol, fpsBuf);
	}
	
	void Render(const Vermilion::RenderData& renderData) {
		Watermark();
		
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