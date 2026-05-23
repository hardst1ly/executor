#pragma once
#include "../globals.h"
#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

class c_gui_window {
public:
    static HWND main_window;
    static HWND edit_control;
    static HWND execute_button;
    static HWND clear_button;
    static HWND script_combo;
    static HWND execute_combo_button;
    static HWND status_label;
    
    static bool is_visible;
    static std::string current_script;

    static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static bool create_window();
    static void show_window();
    static void hide_window();
    static void toggle_window();
    static void execute_script();
    static void clear_editor();
    static void execute_selected_script();
    static void update_status(const char* text);
    
    static void start();
};