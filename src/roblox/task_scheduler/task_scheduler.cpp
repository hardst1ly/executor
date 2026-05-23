#include "task_scheduler.h"
#include "env/env.h"

void rbx::task_scheduler::send_script(const std::string& script) {
	if (script.empty()) return;
	globals::execution_queue.push(script);
}

int rbx::task_scheduler::execution_hook(lua_State* L) {
	if (globals::execution_queue.empty()) return 0;

	auto script = globals::execution_queue.front();
    globals::execution_queue.pop();
    c_execution::execute_script(globals::our_lua_state, script);

	return 0;
}

void rbx::task_scheduler::hook_runservice(lua_State* L) {
	// this is doing game:GetService("RunService").RenderStepped:Connect( -- our hook here )

    lua_getglobal(L, "game");
    lua_getfield(L, -1, "GetService");
    lua_pushvalue(L, -2);

    lua_pushstring(L, "RunService");
    lua_pcall(L, 2, 1, 0);

    lua_getfield(L, -1, "RenderStepped");
    lua_getfield(L, -1, "Connect");
    lua_pushvalue(L, -2);

    lua_pushcclosure(L, execution_hook, nullptr, 0);
    lua_pcall(L, 2, 0, 0);
    lua_pop(L, 2);
}

void rbx::task_scheduler::initialize() {
	globals::roblox_lua_state = (lua_State*)(c_manager::get_lua_state());
	globals::our_lua_state = lua_newthread(globals::roblox_lua_state);
    rbx::c_manager::set_thread_capabilities(globals::our_lua_state, 8, max_caps);
    luaL_sandboxthread(globals::our_lua_state);
	rbx::c_environment::initialize(globals::our_lua_state);
    hook_runservice(globals::our_lua_state);
    send_script("print('exploit initialized ^-^')");
    send_script("print('\"In the end, We are all detected.\" - Sun Tzu, The Art Of War.')");
    auto script = R"(
getgenv().test = "test string"
getgenv().random = 109723
print(getgenv().test)
print(getgenv().random)
print(identifyexecutor())
print(getexecutorname())
)";
    send_script(script);
}