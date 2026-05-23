#include "env.h"

void rbx::c_environment::initialize(lua_State* l) {
    lua_newtable(l);
    lua_setglobal(l, "_G");

    lua_newtable(l);
    lua_setglobal(l, "shared");

	rbx::libraries::misc::register_library(l);

    roblox::print(0, "environment initialized.");
}