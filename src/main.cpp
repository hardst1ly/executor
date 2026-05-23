#include "windows.h"
#include <thread>
#include <chrono>

#include "roblox/task_scheduler/task_scheduler.h"
#include "gui/simple_gui.h"

/*
"In the end, We are all detected."
- Sun Tzu, The Art Of War.
*/

void load() {
	roblox::print(0, "module cuh");
	rbx::task_scheduler::initialize();
	
	// Initialize simple GUI
	std::thread([]() {
		std::this_thread::sleep_for(std::chrono::seconds(1)); // Small delay
		
		if (SimpleGUI::Initialize()) {
			roblox::print(0, "Simple GUI initialized successfully");
		} else {
			roblox::print(0, "Failed to initialize Simple GUI");
		}
	}).detach();

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

LRESULT WINAPI DllMain(HMODULE hmod, ULONG reason, PVOID) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hmod);
		std::thread(load).detach();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		SimpleGUI::Shutdown();
		break;
	}
	return TRUE;
}