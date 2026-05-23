#pragma once
#include "misc/globals.h"

#include "libraries/misc/misc_lib.h"

namespace rbx {
    class c_environment {
    public:
        static void initialize(lua_State* l);

        static void register_global(lua_State* l, const char* global_name, lua_CFunction closure) {
            lua_pushcclosure(l, closure, nullptr, 0);
            lua_setglobal(l, global_name);
        }

        static void register_member(lua_State* l, const char* global_name, lua_CFunction closure) {
            lua_pushcclosure(l, closure, nullptr, 0);
            lua_setfield(l, -2, global_name);
        }

        static void register_alias(lua_State* l, const char* key, std::list<const char*> aliases, bool is_alias_global, bool is_key_global) {
            for (const auto& alias : aliases) {
                if (is_key_global) lua_getglobal(l, key);
                else lua_getfield(l, -1, key);

                if (lua_isnoneornil(l, -1)) {
                    lua_pop(l, 1);
                    return;
                }

                if (is_alias_global) lua_setglobal(l, alias);
                else {
                    bool was_readonly{ false };
                    if (lua_getreadonly(l, -2)) {
                        lua_setreadonly(l, -2, false);
                        was_readonly = true;
                    }
                    lua_setfield(l, -2, alias);
                    if (was_readonly) lua_setreadonly(l, -1, true);
                }
            }
        }
    };
}