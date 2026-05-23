#pragma once
#include <string>
#include <thread>

#include "lua.h"

#include "misc/update/update.h"
#include "misc/globals.h"

#include "roblox/manager/manager.h"
#include "execution/execution.h"

namespace rbx {
	class task_scheduler {
	private:
		static int execution_hook(lua_State* L);
		static void hook_runservice(lua_State* L);
	public:
		static void send_script(const std::string& script);
		static void initialize();
	};
}