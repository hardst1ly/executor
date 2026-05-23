#pragma once
#include <windows.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <queue>
#include <list>

#include <lua.h>
#include <lualib.h>
#include <lstate.h>
#include <lobject.h>

#include "misc/update/update.h"

inline uintptr_t max_caps = ~0ull;

namespace globals {
	inline lua_State* roblox_lua_state;
	inline lua_State* our_lua_state;

	inline std::queue<std::string> execution_queue;
	inline std::queue<std::string> teleport_queue;
}