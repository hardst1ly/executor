#include "pipe_server.h"
#include "../../roblox/task_scheduler/task_scheduler.h"

HANDLE c_pipe_server::pipe_handle = INVALID_HANDLE_VALUE;
bool c_pipe_server::is_running = false;
std::thread c_pipe_server::server_thread;

void c_pipe_server::start_server() {
    if (is_running) return;
    
    is_running = true;
    server_thread = std::thread(handle_client);
    roblox::print(0, "Pipe server started");
}

void c_pipe_server::stop_server() {
    is_running = false;
    if (pipe_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_handle);
        pipe_handle = INVALID_HANDLE_VALUE;
    }
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void c_pipe_server::handle_client() {
    while (is_running) {
        pipe_handle = CreateNamedPipeA(
            "\\\\.\\pipe\\ExecutorPipe",
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            1024,
            1024,
            0,
            nullptr
        );

        if (pipe_handle == INVALID_HANDLE_VALUE) {
            roblox::print(0, "Failed to create pipe");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        roblox::print(0, "Waiting for GUI connection...");
        
        if (ConnectNamedPipe(pipe_handle, nullptr) || GetLastError() == ERROR_PIPE_CONNECTED) {
            roblox::print(0, "GUI connected!");
            
            while (is_running) {
                std::string script = read_message();
                if (!script.empty()) {
                    roblox::print(0, "Received script from GUI");
                    rbx::task_scheduler::send_script(script.c_str());
                } else {
                    break; // Client disconnected
                }
            }
        }

        CloseHandle(pipe_handle);
        pipe_handle = INVALID_HANDLE_VALUE;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::string c_pipe_server::read_message() {
    if (pipe_handle == INVALID_HANDLE_VALUE) return "";

    char buffer[4096];
    DWORD bytes_read = 0;
    
    if (ReadFile(pipe_handle, buffer, sizeof(buffer) - 1, &bytes_read, nullptr)) {
        buffer[bytes_read] = '\0';
        std::string message(buffer);
        
        // Check if it's a command
        if (message.substr(0, 4) == "CMD:") {
            std::string command = message.substr(4);
            roblox::print(0, ("Received command: " + command).c_str());
            
            if (command == "INJECT") {
                roblox::print(0, "Injection command received");
                // Add injection logic here if needed
            }
            
            return ""; // Don't execute commands as scripts
        }
        
        return message;
    }
    
    return "";
}