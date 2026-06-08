// src/rendering/rendering_bindings.cc
#include "rendering/rendering_bindings.h"
#include "ck_rendering.h"

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {
	static int l_clear_rendering(lua_State* L) {
		ck_rendering_clear();
		return 0;
	}

	static const luaL_Reg rendering_lib[] = {
		{ "clear", l_clear_rendering },
		{ nullptr, nullptr }
	};
}

int luaopen_ck_rendering(lua_State* L) {
    luaL_newlib(L, rendering_lib);
    return 1;
}
