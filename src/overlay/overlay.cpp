#include "overlay.h"
#include "../roblox/task_scheduler/task_scheduler.h"

// Include ImGui from deps folder
#include "../../deps/imgui/imgui.h"
#include "../../deps/imgui/imgui_impl_dx11.h"
#include "../../deps/imgui/imgui_impl_win32.h"
#include <dwmapi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

// Static members
HWND ImGuiOverlay::overlay_window = nullptr;
HWND ImGuiOverlay::target_window = nullptr;
ID3D11Device* ImGuiOverlay::device = nullptr;
ID3D11DeviceContext* ImGuiOverlay::device_context = nullptr;
IDXGISwapChain* ImGuiOverlay::swap_chain = nullptr;
ID3D11RenderTargetView* ImGuiOverlay::render_target_view = nullptr;
bool ImGuiOverlay::running = false;
bool ImGuiOverlay::show_gui = true;
std::thread ImGuiOverlay::render_thread;
char ImGuiOverlay::script_buffer[8192] = "print('Hello from ImGui Overlay!')";
bool ImGuiOverlay::connected_to_executor = true;
std::string ImGuiOverlay::status_message = "Ready";

bool ImGuiOverlay::Initialize() {
    if (running) return true;
    
    if (!CreateOverlayWindow()) return false;
    if (!InitializeD3D11()) return false;
    if (!InitializeImGui()) return false;
    
    running = true;
    
    // Set initial transparency state
    ToggleGUI(); // This will set proper transparency
    show_gui = true; // Ensure GUI starts visible
    
    render_thread = std::thread(RenderLoop);
    
    return true;
}

void ImGuiOverlay::Shutdown() {
    running = false;
    
    if (render_thread.joinable()) {
        render_thread.join();
    }
    
    CleanupImGui();
    CleanupD3D11();
    
    if (overlay_window) {
        DestroyWindow(overlay_window);
        overlay_window = nullptr;
    }
}

void ImGuiOverlay::ToggleGUI() {
    show_gui = !show_gui;
    
    // Update window transparency based on GUI state
    if (show_gui) {
        // Make window visible and clickable
        SetWindowLongA(overlay_window, GWL_EXSTYLE, 
            GetWindowLongA(overlay_window, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
        SetLayeredWindowAttributes(overlay_window, 0, 255, LWA_ALPHA);
    } else {
        // Make window transparent and click-through
        SetWindowLongA(overlay_window, GWL_EXSTYLE, 
            GetWindowLongA(overlay_window, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
        SetLayeredWindowAttributes(overlay_window, RGB(0, 0, 0), 0, LWA_COLORKEY);
    }
}

bool ImGuiOverlay::CreateOverlayWindow() {
    // Find Roblox window
    target_window = FindWindowA("WINDOWSCLIENT", nullptr);
    if (!target_window) {
        MessageBoxA(nullptr, "Roblox window not found!", "Error", MB_OK);
        return false;
    }
    
    // Register window class
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // No background brush
    wc.lpszClassName = "ImGuiOverlay";
    
    if (!RegisterClassExA(&wc)) {
        MessageBoxA(nullptr, "Failed to register window class!", "Error", MB_OK);
        return false;
    }
    
    // Get target window rect
    RECT target_rect;
    GetWindowRect(target_window, &target_rect);
    
    // Create overlay window
    overlay_window = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        "ImGuiOverlay",
        "ImGui Overlay",
        WS_POPUP,
        target_rect.left, target_rect.top,
        target_rect.right - target_rect.left,
        target_rect.bottom - target_rect.top,
        nullptr, nullptr,
        GetModuleHandleA(nullptr),
        nullptr
    );
    
    if (!overlay_window) {
        MessageBoxA(nullptr, "Failed to create overlay window!", "Error", MB_OK);
        return false;
    }
    
    // Make window click-through when GUI is hidden
    SetLayeredWindowAttributes(overlay_window, RGB(0, 0, 0), 255, LWA_COLORKEY);
    
    // Extend frame for DWM
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(overlay_window, &margins);
    
    ShowWindow(overlay_window, SW_SHOW);
    UpdateWindow(overlay_window);
    
    return true;
}

bool ImGuiOverlay::InitializeD3D11() {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = overlay_window;
    scd.SampleDesc.Count = 1;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Windowed = TRUE;
    
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        feature_levels, 2, D3D11_SDK_VERSION,
        &scd, &swap_chain, &device, &feature_level, &device_context
    );
    
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create D3D11 device!", "Error", MB_OK);
        return false;
    }
    
    // Create render target view
    ID3D11Texture2D* back_buffer = nullptr;
    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
    if (FAILED(hr)) return false;
    
    hr = device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
    back_buffer->Release();
    
    return SUCCEEDED(hr);
}

bool ImGuiOverlay::InitializeImGui() {
    // Initialize ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplWin32_Init(overlay_window)) return false;
    if (!ImGui_ImplDX11_Init(device, device_context)) return false;
    
    return true;
}

void ImGuiOverlay::RenderLoop() {
    MSG msg;
    
    while (running) {
        // Handle messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Update overlay position to follow target window
        if (target_window) {
            RECT target_rect;
            if (GetWindowRect(target_window, &target_rect)) {
                SetWindowPos(overlay_window, HWND_TOPMOST,
                    target_rect.left, target_rect.top,
                    target_rect.right - target_rect.left,
                    target_rect.bottom - target_rect.top,
                    SWP_NOACTIVATE);
            }
        }
        
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        if (show_gui) {
            RenderGUI();
        }
        
        // Rendering
        ImGui::Render();
        
        // Clear and present
        const float clear_color[4] = { 0.0f, 0.0f, 0.0f, show_gui ? 0.3f : 0.0f }; // Slight background when GUI is shown
        device_context->OMSetRenderTargets(1, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, clear_color);
        
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        swap_chain->Present(1, 0);
        
        Sleep(16); // ~60 FPS
    }
}

void ImGuiOverlay::RenderGUI() {
    // Main window
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Executor", nullptr, ImGuiWindowFlags_None)) {
        // Script editor
        ImGui::Text("Script Editor:");
        ImGui::InputTextMultiline("##script", script_buffer, sizeof(script_buffer), ImVec2(580, 250));
        
        // Buttons
        if (ImGui::Button("Execute", ImVec2(100, 30))) {
            ExecuteScript();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Clear", ImVec2(100, 30))) {
            ClearScript();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Infinite Yield", ImVec2(120, 30))) {
            strcpy_s(script_buffer, "loadstring(game:HttpGet('https://raw.githubusercontent.com/EdgeIY/infiniteyield/master/source'))()");
        }
        
        // Status
        ImGui::Text("Status: %s", status_message.c_str());
        ImGui::Text(connected_to_executor ? "Connected to executor" : "Not connected");
    }
    ImGui::End();
}

void ImGuiOverlay::ExecuteScript() {
    if (strlen(script_buffer) == 0) {
        status_message = "No script to execute!";
        return;
    }
    
    SendScript(std::string(script_buffer));
    status_message = "Script executed!";
}

void ImGuiOverlay::ClearScript() {
    memset(script_buffer, 0, sizeof(script_buffer));
    status_message = "Script cleared!";
}

void ImGuiOverlay::SendScript(const std::string& script) {
    // Send to task scheduler
    rbx::task_scheduler::send_script(script.c_str());
}

void ImGuiOverlay::CleanupD3D11() {
    if (render_target_view) { render_target_view->Release(); render_target_view = nullptr; }
    if (swap_chain) { swap_chain->Release(); swap_chain = nullptr; }
    if (device_context) { device_context->Release(); device_context = nullptr; }
    if (device) { device->Release(); device = nullptr; }
}

void ImGuiOverlay::CleanupImGui() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

LRESULT CALLBACK ImGuiOverlay::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;
    
    switch (msg) {
    case WM_KEYDOWN:
        if (wparam == VK_INSERT) {
            ToggleGUI();
        }
        break;
    case WM_DESTROY:
        running = false;
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProc(hwnd, msg, wparam, lparam);
}