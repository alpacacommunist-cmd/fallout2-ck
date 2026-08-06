#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

#include <lua.hpp>
#include <algorithm>
#include "picojson.h"

#include "ck_log.h"
static const Logger proxy_log("CK State Lua Proxy");

extern lua_State* gLuaState;
extern const char* g_current_mod_id;

namespace ck::proxy::detail {
    extern int get_state_data;
    extern int get_proto_list;
    extern int receive_proto_list;

	extern int state_sync_load;
    extern int state_sync_save;
}

static bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

static void picojson_to_lua(lua_State* L, const picojson::value& val) {
    if (val.is<double>()) lua_pushnumber(L, val.get<double>());
    else if (val.is<bool>()) lua_pushboolean(L, val.get<bool>());
    else if (val.is<std::string>()) lua_pushstring(L, val.get<std::string>().c_str());
    else if (val.is<picojson::object>()) {
        lua_newtable(L);
        const picojson::object& obj = val.get<picojson::object>();

        for (const auto& [key, value] : obj) {
            if (is_number(key)) lua_pushinteger(L, std::stoll(key));
            else lua_pushstring(L, key.c_str());

            picojson_to_lua(L, value);
            lua_settable(L, -3);
        }
    } else {
        lua_pushnil(L);
    }
}

static picojson::value lua_to_picojson(lua_State* L, int idx) {
    int t = lua_type(L, idx);

    if (t == LUA_TNUMBER) {
        return picojson::value(static_cast<double>(lua_tonumber(L, idx)));
    } else if (t == LUA_TBOOLEAN) {
        return picojson::value((bool)lua_toboolean(L, idx));
    } else if (t == LUA_TSTRING) {
        return picojson::value(std::string(lua_tostring(L, idx)));
    } else if (t == LUA_TTABLE) {
        picojson::object obj;

        lua_pushnil(L);
        while (lua_next(L, idx < 0 ? idx - 1 : idx) != 0) {
            std::string key;
            if (lua_type(L, -2) == LUA_TNUMBER) {
                key = std::to_string(lua_tointeger(L, -2));
            } else {
                key = lua_tostring(L, -2);
            }

            obj[key] = lua_to_picojson(L, -1);
            lua_pop(L, 1);
        }
        return picojson::value(obj);
    }

    return picojson::value();
}

namespace ck::proxy {
    void internal_iterate_lua_array(std::function<void()> element_callback) {
        if (!lua_istable(gLuaState, -1)) return;

        lua_pushnil(gLuaState);

        while (lua_next(gLuaState, -2) != 0) {
            element_callback();
            lua_pop(gLuaState, 1);
        }
    }

	template<typename... Args>
    ObjectState execute_proxy_call_state(int func_ref, Args... args) {
        LuaStackGuard guard;
        ObjectState result;

        if (!internal_call_start(func_ref)) return result;
        (push_arg(args), ...);
        if (!internal_call_execute(sizeof...(Args), 1)) return result;

        if (lua_istable(gLuaState, -1)) {
            result.tile = read_table_int("tile", -1);
            result.hp   = read_table_int("hp", -1);
        }

        return result;
    }

    std::vector<CustomProtoState> get_proto_list() {
        auto parse_proto = []() -> CustomProtoState {
            CustomProtoState state;
            state.id  = read_table_int("id", 0);
            state.tag = read_table_string("tag", "");
            return state;
        };

        return execute_proxy_call_vector<CustomProtoState>(detail::get_proto_list, parse_proto);
    }

	ObjectState get_object_state(int map_id, const std::string& lua_tag) {
		return execute_proxy_call_state(detail::get_state_data, g_current_mod_id, map_id, lua_tag);
	}

	bool sync_state_load(const picojson::value& state_data) {
		LuaStackGuard guard;

        if (!internal_call_start(detail::state_sync_load)) return false;
        picojson_to_lua(gLuaState, state_data);

        if (!internal_call_execute(1, 0)) return false;

        return true;
    }

    picojson::value sync_state_save() {
		LuaStackGuard guard;

        picojson::value result{picojson::object()};

        if (!internal_call_start(detail::state_sync_save)) return result;
        if (!internal_call_execute(0, 1)) return result;

        if (lua_istable(gLuaState, -1)) {
            result = lua_to_picojson(gLuaState, -1);
        } else {
            proxy_log.error("PicoJSON Save Proxy: Lua hook did not return a valid state table!");
        }

        lua_pop(gLuaState, 1);
        return result;
    }
}
