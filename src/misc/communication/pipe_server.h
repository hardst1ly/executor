#pragma once
#include "../globals.h"
#include <windows.h>
#include <string>
#include <thread>

class c_pipe_server {
public:
    static HANDLE pipe_handle;
    static bool is_running;
    static std::thread server_thread;

    static void start_server();
    static void stop_server();
    static void handle_client();
    static std::string read_message();
};