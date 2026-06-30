#ifndef CK_LUA_PROXY_STATE_H
#define CK_LUA_PROXY_STATE_H

#include "ck_lua_proxy/ck_lua_proxy.h"
#include <string>

namespace picojson { class value; }

namespace ck::proxy {
    int get_state_tile(const int map_id, const std::string& lua_tag);

	bool sync_state_load(const picojson::value& state_data);
    picojson::value sync_state_save();
}

#endif
