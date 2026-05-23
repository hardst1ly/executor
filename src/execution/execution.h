#pragma once
#include "misc/globals.h"

#include "luacode.h"
#include "luau/BytecodeBuilder.h"
#include "luau/BytecodeUtils.h"
#include "luau/Compiler.h"

#include "roblox/manager/manager.h"

class bytecode_encoder : public Luau::BytecodeEncoder {
    inline void encode(uint32_t* data, size_t count) override {
        for (size_t i = 0; i < count;) {
            uint8_t op = LUAU_INSN_OP(data[i]);
            auto op_len = Luau::getOpLength((LuauOpcode)(op));
            auto table = (BYTE*)(update::lua::opcode_lookup_table);
            uint8_t new_op = op * 227;
            new_op = table[new_op];
            data[i] = (new_op) | (data[i] & ~0xff);
            i += op_len;
        }
    }
};

class c_execution {
private:
    static inline bytecode_encoder encoder;
	static std::string compile_script(const std::string& script);
public:
	static void execute_script(lua_State* L, const std::string& script);
};