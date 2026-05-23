#include "misc_lib.h"
#include "env/env.h"

int identifyexecutor(lua_State* l) {
	lua_pushstring(l, "skid");
	lua_pushstring(l, "1.0.0");
	return 2;
}

int getexecutorname(lua_State* l) {
	lua_pushstring(l, "skid");
	return 1;
}

int getgenv(lua_State* l) {
	lua_pushvalue(l, LUA_ENVIRONINDEX);
	return 1;
}

void rbx::libraries::misc::register_library(lua_State* l) {
	rbx::c_environment::register_global(l, "identifyexecutor", identifyexecutor);
	rbx::c_environment::register_global(l, "getexecutorname", getexecutorname);
	rbx::c_environment::register_global(l, "getgenv", getgenv);
}