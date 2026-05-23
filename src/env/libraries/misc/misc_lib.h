#pragma once
#include "misc/globals.h"

namespace rbx {
	namespace libraries {
		class misc {
		public:
			static void register_library(lua_State* l);
		};
	}
}