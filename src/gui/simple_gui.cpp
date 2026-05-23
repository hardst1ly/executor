#include "simple_gui.h"
#include "../roblox/task_scheduler/task_scheduler.h"

#define ID_SCRIPT_EDIT 1001
#define ID_EXECUTE_BTN 1002
#define ID_CLEAR_BTN 1003

// Static members
HWND SimpleGUI::main_window = NULL;
HWND SimpleGUI::script_edit = NULL;
HWND SimpleGUI::execute_button = NULL;
HWND SimpleGUI::clear_button = NULL;
HWND SimpleGUI::status_label = NULL;
bool SimpleGUI::running = false;
bool SimpleGUI::visible = false;
std::thread SimpleGUI::gui_thread;

bool SimpleGUI::Initialize() {
    if (running) return true;
    
    running = true;
    gui_thread = std::thread(RunMessageLoop);
    
    return true;
}

void SimpleGUI::Shutdown() {
    running = false;
    
    if (main_window) {
        PostMessage(main_window, WM_CLOSE, 0, 0);
    }
    
    if (gui_thread.joinable()) {
        gui_thread.join();
    }
}

void SimpleGUI::Toggle() {
    if (main_window) {
        visible = !visible;
        ShowWindow(main_window, visible ? SW_SHOW : SW_HIDE);
        if (visible) {
            SetForegroundWindow(main_window);
        }
    }
}

bool SimpleGUI::CreateGUIWindow() {
    // Register window class
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "SimpleExecutorGUI";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassA(&wc)) {
        return false;
    }
    
    // Create main window
    main_window = CreateWindowA(
        "SimpleExecutorGUI",
        "Executor GUI - INSERT to toggle",
        WS_OVERLAPPEDWINDOW,
        100, 100, 600, 450,
        NULL, NULL,
        GetModuleHandleA(NULL),
        NULL
    );
    
    if (!main_window) {
        return false;
    }
    
    // Create script editor
    script_edit = CreateWindowA(
        "EDIT",
        "print('Hello from Simple GUI!')",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        10, 10, 560, 300,
        main_window, (HMENU)ID_SCRIPT_EDIT,
        GetModuleHandleA(NULL), NULL
    );
    
    // Create execute button
    execute_button = CreateWindowA(
        "BUTTON", "Execute",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 320, 100, 30,
        main_window, (HMENU)ID_EXECUTE_BTN,
        GetModuleHandleA(NULL), NULL
    );
    
    // Create clear button
    clear_button = CreateWindowA(
        "BUTTON", "Clear",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        120, 320, 100, 30,
        main_window, (HMENU)ID_CLEAR_BTN,
        GetModuleHandleA(NULL), NULL
    );
    
    // Create status label
    status_label = CreateWindowA(
        "STATIC", "Ready",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 360, 560, 20,
        main_window, NULL,
        GetModuleHandleA(NULL), NULL
    );
    
    // Set font for all controls
    HFONT font = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    
    SendMessage(script_edit, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessage(execute_button, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessage(clear_button, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessage(status_label, WM_SETFONT, (WPARAM)font, TRUE);
    
    // Show window
    visible = true;
    ShowWindow(main_window, SW_SHOW);
    UpdateWindow(main_window);
    
    return true;
}

void SimpleGUI::RunMessageLoop() {
    if (!CreateGUIWindow()) {
        running = false;
        return;
    }
    
    MSG msg;
    while (running && GetMessage(&msg, NULL, 0, 0)) {
        // Handle INSERT key globally
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_INSERT) {
            Toggle();
            continue;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    running = false;
}

void SimpleGUI::ExecuteScript() {
    if (!script_edit) return;
    
    int length = GetWindowTextLengthA(script_edit);
    if (length == 0) {
        UpdateStatus("No script to execute!");
        return;
    }
    
    char* buffer = new char[length + 1];
    GetWindowTextA(script_edit, buffer, length + 1);
    
    // Send to task scheduler
    rbx::task_scheduler::send_script(buffer);
    UpdateStatus("Script executed!");
    
    delete[] buffer;
}

void SimpleGUI::ClearScript() {
    if (script_edit) {
        SetWindowTextA(script_edit, "");
        UpdateStatus("Script cleared!");
    }
}

void SimpleGUI::UpdateStatus(const std::string& message) {
    if (status_label) {
        SetWindowTextA(status_label, message.c_str());
    }
}

LRESULT CALLBACK SimpleGUI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_EXECUTE_BTN:
            ExecuteScript();
            break;
        case ID_CLEAR_BTN:
            ClearScript();
            break;
        }
        break;
        
    case WM_KEYDOWN:
        if (wparam == VK_INSERT) {
            Toggle();
        }
        break;
        
    case WM_CLOSE:
        visible = false;
        ShowWindow(hwnd, SW_HIDE);
        return 0; // Don't destroy, just hide
        
    case WM_DESTROY:
        running = false;
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProc(hwnd, msg, wparam, lparam);
}