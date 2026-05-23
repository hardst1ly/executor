#include "execution.h"

std::string c_execution::compile_script(const std::string& script) {
    const char* mutable_globals[] = { "Game", "Workspace", "game", "plugin", "script", "shared", "workspace", "_G", "_ENV", nullptr };

    Luau::CompileOptions options;
    options.mutableGlobals = mutable_globals;
    options.vectorLib = "Vector3";
    options.vectorCtor = "new";
    options.vectorType = "Vector3";

    return Luau::compile(script, options, {}, &encoder);
}

void c_execution::execute_script(lua_State* L, const std::string& script) {
    if (script.empty()) return;

    auto og_top = lua_gettop(L);
    auto thread = lua_newthread(L);
    lua_pop(L, 1);
    luaL_sandboxthread(thread);
    rbx::c_manager::set_thread_capabilities(thread, 8, max_caps);

    auto bytecode = compile_script(script);
	if (bytecode.empty()) return;
    
    if (luau_load(thread, "@skid", bytecode.data(), bytecode.size(), 0) != LUA_OK) {
        auto err = lua_tostring(thread, -1);
		roblox::print(3, "%s", err);
        lua_pop(thread, 1);
        lua_settop(L, og_top);
        return;
    }

    auto closure = (Closure*)(lua_topointer(thread, -1));
    if (closure && closure->l.p)
        rbx::c_manager::set_proto_capabilities(closure->l.p, &max_caps);

    lua_getglobal(L, "task");
    lua_getfield(L, -1, "defer");
    lua_remove(L, -2);
    lua_xmove(thread, L, 1);

    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        auto err = lua_tostring(L, -1);
        roblox::print(3, "%s", err);
        lua_pop(L, 1);
    }

    lua_settop(thread, 0);
    lua_settop(L, og_top);
}