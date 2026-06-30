#ifndef CK_LUA_PROXY_STATE_H
#define CK_LUA_PROXY_STATE_H

#include "ck_lua_proxy/ck_lua_proxy.h"
#include <string>

namespace ck::proxy {
    int execute_get_state_tile(const std::string& mod_id, int map_id, const std::string& lua_tag);
}

#endif
