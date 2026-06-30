#ifndef CK_LUA_PROXY_H
#define CK_LUA_PROXY_H

#include <string>

void ck_lua_proxy_init();
void ck_lua_proxy_shutdown();

namespace ck::proxy {
	bool is_ready();

	void push_arg(int val);
	void push_arg(unsigned int val);
	void push_arg(double val);
	void push_arg(const char* val);
	void push_arg(const std::string& val);
	void push_arg(bool val);

	int         read_table_int(const char* key, int default_val = -1);
	bool        read_table_bool(const char* key, bool default_val = false);
	std::string read_table_string(const char* key, const std::string& default_val = "");

	struct LuaStackGuard {
		int initial_top;
		LuaStackGuard();
		~LuaStackGuard();

		LuaStackGuard(const LuaStackGuard&) = delete;
		LuaStackGuard& operator=(const LuaStackGuard&) = delete;
	};

	bool internal_call_start(int func_ref);
	bool internal_call_execute(int nargs, int nresults);

	int         internal_pop_int();
	bool        internal_pop_bool();
	std::string internal_pop_string();

	template<typename ReturnType, typename... Args>
	ReturnType execute_proxy_call(int func_ref, Args... args) {
		LuaStackGuard guard;
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
}

#endif
