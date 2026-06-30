#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

namespace ck::proxy {
    int execute_get_state_tile(const std::string& mod_id, int map_id, const std::string& lua_tag) {
        return execute_proxy_call<int>(ck::proxy::get_state_tile, mod_id, map_id, lua_tag);
    }
}
