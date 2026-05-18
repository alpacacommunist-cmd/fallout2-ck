#include <stdio.h>

#include "vendor/luajit/src/lua.h"
#include "vendor/luajit/src/lualib.h"
#include "vendor/luajit/src/lauxlib.h"

int init_lua_system() {
    // Lua state
    lua_State *L = luaL_newstate();
    if (L == NULL) {
        return -1; // Initialize error
    }

    // (math, table, string, io etc)
    luaL_openlibs(L);
    // Ready

	// Quick check
	int script_executed = luaL_dofile(L, "mods/username/test.lua");
	if (script_executed != 0) {
		printf("Lua Error: %s\n", lua_tostring(L, -1));
		lua_close(L);

		return -2;
	}
	
    
    // Close state on exit
    lua_close(L);
    return 0;
}

int main() {
    printf("Starting Fallout 2 Construction Kit...\n");

    int result = init_lua_system(); 

    if (result != 0) {
        printf("Engine failed to start!\n");
        return 1;
    }

    printf("Engine shut down cleanly. Bye!\n");
    return 0;
}
