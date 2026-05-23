#include "manager.h"

uintptr_t rbx::c_manager::get_datamodel() {
	auto fake_dm_ptr = *(uintptr_t*)(update::datamodel::fake_datamodel_pointer);
	return *(uintptr_t*)(fake_dm_ptr + update::datamodel::fake_datamodel_to_datamodel);
}

uintptr_t rbx::c_manager::get_script_context() {
	uintptr_t children = *(uintptr_t*)(get_datamodel() + update::instance::children);
	uintptr_t script_context = *(uintptr_t*)(children) + update::datamodel::script_context;
	return *(uintptr_t*)(script_context);
}

uintptr_t rbx::c_manager::get_lua_state() {
	auto es = get_script_context() + 0x4f0;
	auto enc = (uint32_t*)(es + 0x148);
	uint32_t low = enc[0] - (uint32_t)((uintptr_t)(enc));
	uint32_t high = enc[1] - (uint32_t)((uintptr_t)(enc));
	return ((uint64_t)(high) << 32) | low;
}

void rbx::c_manager::set_proto_capabilities(Proto* proto, uintptr_t* c) {
	if (!proto) return;
	proto->userdata = c;
	for (int i = 0; i < proto->sizep; ++i)
		set_proto_capabilities(proto->p[i], c);
}

void rbx::c_manager::set_thread_capabilities(lua_State* L, int i, uintptr_t c) {
	/*
	*(int*)((uintptr_t)(L->userdata) + 0x30) = i;
	*(uintptr_t*)((uintptr_t)(L->userdata) + 0x50) = c;
	*/
	L->userdata->identity = i;
	L->userdata->capabilities = c;
}