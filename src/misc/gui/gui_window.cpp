#include "gui_window.h"
#include "../../roblox/task_scheduler/task_scheduler.h"

HWND c_gui_window::main_window = nullptr;
HWND c_gui_window::edit_control = nullptr;
HWND c_gui_window::execute_button = nullptr;
HWND c_gui_window::clear_button = nullptr;
HWND c_gui_window::script_combo = nullptr;
HWND c_gui_window::execute_combo_button = nullptr;
HWND c_gui_window::status_label = nullptr;

bool c_gui_window::is_visible = false;
std::string c_gui_window::current_script = "";

const char* script_names[] = {
    "Infinite Yield",
    "UNC Test",
    "SUNC Test", 
    "Vulnerability Test"
};

const char* script_sources[] = {
    "loadstring(game:HttpGet(\"https://raw.githubusercontent.com/EdgeIY/infiniteyield/master/source\"))()",
    "loadstring(game:HttpGet(\"https://raw.githubusercontent.com/unified-naming-convention/NamingStandard/refs/heads/main/UNCCheckEnv.lua\"))()",
    "getgenv().sUNCDebug = {\n    [\"printcheckpoints\"] = false,\n    [\"delaybetweentests\"] = 0,\n    [\"printtesttimetaken\"] = false,\n}\n\nloadstring(game:HttpGet(\"https://script.sunc.su/\"))()",
    "loadstring(game:HttpGet(\"https://raw.githubusercontent.com/zryr/Vulnerability-Check/refs/heads/main/Script\"))()"
};

#define ID_EXECUTE_BUTTON 1001
#define ID_CLEAR_BUTTON 1002
#define ID_SCRIPT_COMBO 1003
#define ID_EXECUTE_COMBO_BUTTON 1004
#define ID_EDIT_CONTROL 1005

LRESULT CALLBACK c_gui_window::wnd_proc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_CREATE: {
        // Create edit control (multiline textbox)
        edit_control = CreateWindowExA(
            WS_EX_CLIENTEDGE,
            "EDIT",
            "print('Hello from GUI!')",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            10, 10, 560, 300,
            hWnd, (HMENU)ID_EDIT_CONTROL, GetModuleHandle(nullptr), nullptr
        );

        // Create execute button
        execute_button = CreateWindowA(
            "BUTTON", "Execute",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 320, 100, 30,
            hWnd, (HMENU)ID_EXECUTE_BUTTON, GetModuleHandle(nullptr), nullptr
        );

        // Create clear button
        clear_button = CreateWindowA(
            "BUTTON", "Clear",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 320, 100, 30,
            hWnd, (HMENU)ID_CLEAR_BUTTON, GetModuleHandle(nullptr), nullptr
        );

        // Create script combo box
        script_combo = CreateWindowA(
            "COMBOBOX", "",
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            10, 360, 200, 200,
            hWnd, (HMENU)ID_SCRIPT_COMBO, GetModuleHandle(nullptr), nullptr
        );

        // Add items to combo box
        for (int i = 0; i < 4; i++) {
            SendMessageA(script_combo, CB_ADDSTRING, 0, (LPARAM)script_names[i]);
        }
        SendMessageA(script_combo, CB_SETCURSEL, 0, 0);

        // Create execute combo button
        execute_combo_button = CreateWindowA(
            "BUTTON", "Execute Selected",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            220, 360, 120, 25,
            hWnd, (HMENU)ID_EXECUTE_COMBO_BUTTON, GetModuleHandle(nullptr), nullptr
        );

        // Create status label
        status_label = CreateWindowA(
            "STATIC", "Ready",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 400, 560, 20,
            hWnd, nullptr, GetModuleHandle(nullptr), nullptr
        );

        // Set font for all controls
        HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
        
        SendMessage(edit_control, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(execute_button, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(clear_button, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(script_combo, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(execute_combo_button, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(status_label, WM_SETFONT, (WPARAM)hFont, TRUE);

        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wparam)) {
        case ID_EXECUTE_BUTTON:
            execute_script();
            break;
        case ID_CLEAR_BUTTON:
            clear_editor();
            break;
        case ID_EXECUTE_COMBO_BUTTON:
            execute_selected_script();
            break;
        }
        break;
    }
    case WM_KEYDOWN: {
        if (wparam == VK_INSERT) {
            toggle_window();
        }
        break;
    }
    case WM_CLOSE:
        hide_window();
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wparam, lparam);
}

bool c_gui_window::create_window() {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "ExecutorGUI";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK);
        return false;
    }

    main_window = CreateWindowA(
        "ExecutorGUI",
        "Executor GUI - INSERT to toggle",
        WS_OVERLAPPEDWINDOW,
        100, 100, // Fixed position instead of CW_USEDEFAULT
        600, 460,
        nullptr, nullptr,
        GetModuleHandle(nullptr), nullptr
    );

    if (!main_window) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK);
        return false;
    }

    MessageBoxA(NULL, "Window created successfully", "Debug", MB_OK);
    return true;
}

void c_gui_window::show_window() {
    if (main_window) {
        ShowWindow(main_window, SW_SHOW);
        SetForegroundWindow(main_window);
        is_visible = true;
        update_status("Window shown");
    }
}

void c_gui_window::hide_window() {
    if (main_window) {
        ShowWindow(main_window, SW_HIDE);
        is_visible = false;
        update_status("Window hidden");
    }
}

void c_gui_window::toggle_window() {
    if (is_visible) {
        hide_window();
    } else {
        show_window();
    }
}

void c_gui_window::execute_script() {
    if (!edit_control) return;

    int length = GetWindowTextLengthA(edit_control);
    if (length == 0) {
        update_status("No script to execute");
        return;
    }

    char* buffer = new char[length + 1];
    GetWindowTextA(edit_control, buffer, length + 1);
    
    rbx::task_scheduler::send_script(buffer);
    update_status("Script executed");
    
    delete[] buffer;
}

void c_gui_window::clear_editor() {
    if (edit_control) {
        SetWindowTextA(edit_control, "");
        update_status("Editor cleared");
    }
}

void c_gui_window::execute_selected_script() {
    if (!script_combo) return;

    int selected = SendMessageA(script_combo, CB_GETCURSEL, 0, 0);
    if (selected >= 0 && selected < 4) {
        rbx::task_scheduler::send_script(script_sources[selected]);
        update_status("Selected script executed");
    }
}

void c_gui_window::update_status(const char* text) {
    if (status_label) {
        SetWindowTextA(status_label, text);
    }
}

void c_gui_window::start() {
    if (!create_window()) {
        return;
    }

    // Show window initially
    show_window();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}