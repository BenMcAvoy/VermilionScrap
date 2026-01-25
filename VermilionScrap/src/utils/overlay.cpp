#include "overlay.h"

#include "sig.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace {
    using namespace Vermilion;
    
    RenderCallback_T RenderCallback = nullptr;
    WndProcCallback_T WndProcCallback = nullptr;
    RenderData RenderData = {};
    
    typedef HRESULT(__stdcall* DXGIPresent_T)(IDXGISwapChain* pSwapChain, UINT syncInterval, UINT flags);
    DXGIPresent_T OriginalPresent = nullptr;
    LONG_PTR OriginalWndProc = 0;

    // Present hook
    HRESULT __stdcall HookedPresent(IDXGISwapChain* pSwapChain, const UINT syncInterval, const UINT flags) {
        // Setup RenderData on first call
        if (!RenderData.pDevice) {
            // Get device, context, and render target view
            ID3D11Device* pDevice = nullptr;
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&pDevice)))) {
                RenderData.pDevice = reinterpret_cast<IDXGIDevice*>(pDevice);
                pDevice->GetImmediateContext(&RenderData.pContext);
                // Get back buffer and create render target view
                ID3D11Texture2D* pBackBuffer = nullptr;
                if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)))) {
                    pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &RenderData.pRenderTargetView);
                    pBackBuffer->Release();
                }
            }
            
            RenderData.pSwapChain = pSwapChain;
            
            // Initialize ImGui
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            
            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 14.0f);
            io.Fonts->Build();
           
            ImGui_ImplDX11_Init(
                reinterpret_cast<ID3D11Device*>(RenderData.pDevice),
                RenderData.pContext
            );
            // Get window handle from swap chain
            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            pSwapChain->GetDesc(&swapChainDesc);
            ImGui_ImplWin32_Init(swapChainDesc.OutputWindow);
        }
        
        // set render target
        RenderData.pContext->OMSetRenderTargets(1, &RenderData.pRenderTargetView, nullptr);
		
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        
        ImGui::NewFrame();
        
        if (RenderCallback)
            RenderCallback(RenderData);
        
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        return OriginalPresent(pSwapChain, syncInterval, flags);
    }
    
    // WndProc hook
    LRESULT CALLBACK HookedWndProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
        if (WndProcCallback) {
            const auto res = WndProcCallback(hWnd, uMsg, wParam, lParam);
            if (res != 0) {
                return res;
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
        static SigScanner scanner("GameOverlayRenderer64.dll");
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
}

namespace Vermilion {
    void HookOverlay(const RenderCallback_T& callback, const WndProcCallback_T& wndProcCallback) {
        RenderCallback = callback;
        WndProcCallback = wndProcCallback;
        
        SetPresent(HookedPresent);
        SetWndProc(HookedWndProc);
    }
    
    void ShutdownOverlay() {
        SetPresent(OriginalPresent);
        SetWndProc(reinterpret_cast<WNDPROC>(OriginalWndProc));
		Sleep(100);
    }
}