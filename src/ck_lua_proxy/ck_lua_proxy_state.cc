#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

extern const char* g_current_mod_id;

namespace ck::proxy::detail {
    extern int get_state_tile;
}

namespace ck::proxy {
    int get_state_tile(int map_id, const std::string& lua_tag) {
        return execute_proxy_call<int>(detail::get_state_tile, g_current_mod_id, map_id, lua_tag);
    }
}
