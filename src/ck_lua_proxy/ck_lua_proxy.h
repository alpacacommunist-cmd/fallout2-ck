#ifndef CK_LUA_PROXY_H
#define CK_LUA_PROXY_H

#include <string>

void ck_lua_proxy_init();
void ck_lua_proxy_shutdown();

namespace ck::proxy {
    extern int emit_for_mod;
    extern int on_map_update;
    extern int on_proc;
    extern int clear_tracked_objects;
    extern int clear_registry;
    extern int load_and_init_mod;
    extern int get_state_tile;
    extern int state_sync_load;
    extern int state_sync_save;

	bool is_ready();

	void push_arg(int val);
	void push_arg(unsigned int val);
	void push_arg(double val);
	void push_arg(const char* val);
	void push_arg(const std::string& val);
	void push_arg(bool val);

	bool internal_call_start(int func_ref);
	bool internal_call_execute(int nargs, int nresults);

	int         internal_pop_int();
	bool        internal_pop_bool();
	std::string internal_pop_string();

	template<typename ReturnType, typename... Args>
	ReturnType execute_proxy_call(int func_ref, Args... args) {
		if (!internal_call_start(func_ref)) return ReturnType{};

		(push_arg(args), ...);

		int total_args = sizeof...(Args);
		if (!internal_call_execute(total_args, 1)) {
			return ReturnType{};
		}

		ReturnType result{};
		if constexpr (std::is_same_v<ReturnType, int>) {
			result = internal_pop_int();
		} else if constexpr (std::is_same_v<ReturnType, bool>) {
			result = internal_pop_bool();
		} else if constexpr (std::is_same_v<ReturnType, std::string>) {
			result = internal_pop_string();
		}

		return result;
	}

	int execute_get_state_tile(int map_id, const std::string& lua_tag);

	void execute_sync_load();
	void execute_sync_save();
}

#endif
