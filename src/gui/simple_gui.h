#pragma once
#include <windows.h>
#include <string>
#include <thread>

class SimpleGUI {
private:
    static HWND main_window;
    static HWND script_edit;
    static HWND execute_button;
    static HWND clear_button;
    static HWND status_label;
    static bool running;
    static bool visible;
    static std::thread gui_thread;

public:
    static bool Initialize();
    static void Shutdown();
    static void Toggle();
    static bool IsRunning() { return running; }

private:
    static bool CreateGUIWindow();
    static void RunMessageLoop();
    static void ExecuteScript();
    static void ClearScript();
    static void UpdateStatus(const std::string& message);
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};