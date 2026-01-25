#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include <functional>

namespace Vermilion {
    struct RenderData {
        IDXGISwapChain* pSwapChain;
        IDXGIDevice* pDevice;
        ID3D11DeviceContext* pContext;
        ID3D11RenderTargetView* pRenderTargetView;
    };
    
    using RenderCallback_T = std::function<void(const RenderData&)>;
    using WndProcCallback_T = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;
    
    void HookOverlay(const RenderCallback_T& callback, const WndProcCallback_T& wndProcCallback);
    void ShutdownOverlay();
}