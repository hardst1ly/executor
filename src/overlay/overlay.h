#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <thread>
#include <string>

class ImGuiOverlay {
private:
    static HWND overlay_window;
    static HWND target_window;
    static ID3D11Device* device;
    static ID3D11DeviceContext* device_context;
    static IDXGISwapChain* swap_chain;
    static ID3D11RenderTargetView* render_target_view;
    static bool running;
    static bool show_gui;
    static std::thread render_thread;
    
    // GUI state
    static char script_buffer[8192];
    static bool connected_to_executor;
    static std::string status_message;

public:
    static bool Initialize();
    static void Shutdown();
    static void ToggleGUI();
    static bool IsRunning() { return running; }
    
private:
    static bool CreateOverlayWindow();
    static bool InitializeD3D11();
    static bool InitializeImGui();
    static void RenderLoop();
    static void RenderGUI();
    static void CleanupD3D11();
    static void CleanupImGui();
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Communication
    static void SendScript(const std::string& script);
    static void ExecuteScript();
    static void ClearScript();
};