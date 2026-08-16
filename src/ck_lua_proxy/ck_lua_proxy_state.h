#ifndef CK_LUA_PROXY_STATE_H
#define CK_LUA_PROXY_STATE_H

#include <vector>
#include <functional>
#include <string>

#include "ck_lua_proxy/ck_lua_proxy.h"

namespace picojson { class value; }

namespace ck::proxy {
	struct ObjectState { int tile = -1; int elevation = -1; int hp = -1; int id = -1; bool is_dead = false; };
    struct CustomProtoState { int id = 0; std::string tag; };

    template <typename T, typename... Args>
    std::vector<T> execute_proxy_call_vector(int func_ref, std::function<T()> parser, Args... args) {
        LuaStackGuard guard;
        std::vector<T> result;

        if (!internal_call_start(func_ref)) return result;
        (push_arg(args), ...);
        if (!internal_call_execute(sizeof...(Args), 1)) return result;

        internal_iterate_lua_array([&]() { result.push_back(parser()); });

        return result;
    }

    ObjectState get_object_state(int map_id, const std::string& lua_tag);
    std::vector<CustomProtoState> get_proto_list();

	bool sync_state_load(const picojson::value& state_data);
    picojson::value sync_state_save();

	template<typename... Args>
    ObjectState execute_proxy_call_state(int func_ref, Args... args);
}

#endif
