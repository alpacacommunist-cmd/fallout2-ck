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
    
    // Close state on exit
    lua_close(L);
    return 0;
}
