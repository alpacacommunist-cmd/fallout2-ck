#include "picojson.h"
#include "ck_utils.h"
#include "ck_state/ck_state.h"

#include <fstream>
#include <string>
#include <algorithm>

static picojson::value g_game_state;

#include "ck_log.h"
static const Logger state_log("CK State");

void dump_json_to_log(const picojson::value& val, int indent = 0) {
	std::string spaces(indent * 2, ' ');

	if (val.is<picojson::object>()) {
		const picojson::object& obj = val.get<picojson::object>();
		for (const auto& [key, value] : obj) {
			if (value.is<picojson::object>()) {
				state_log.info("{}{}: {{", spaces, key);
				dump_json_to_log(value, indent + 1);
				state_log.info("{}}}", spaces);
			}

			else if (value.is<double>()) {
				state_log.info("{}{}: {} (number)", spaces, key, value.get<double>());
			}
			else if (value.is<std::string>()) {
				state_log.info("{}{}: \"{}\"", spaces, key, value.get<std::string>());
			}
			else if (value.is<bool>()) {
				state_log.info("{}{}: {}", spaces, key, value.get<bool>() ? "true" : "false");
			}
		}
	}
}

bool is_number(const std::string& s) {
	return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void picojson_to_lua(lua_State* L, const picojson::value& val) {
	if (val.is<double>()) {
		lua_pushnumber(L, val.get<double>());
	} else if (val.is<bool>()) {
		lua_pushboolean(L, val.get<bool>());
	} else if (val.is<std::string>()) {
		lua_pushstring(L, val.get<std::string>().c_str());
	} else if (val.is<picojson::object>()) {
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

bool ck_state_load(const char* path) {
	if (path == nullptr || gLuaState == nullptr) return false;

	std::string clean_path(path);
	for (char& c : clean_path) {
		if (c == '\\') c = '/';
	}
	size_t dot_pos = clean_path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		clean_path = clean_path.substr(0, dot_pos) + ".json";
	}

	std::ifstream file(clean_path);
	if (!file.is_open()) {
		state_log.warn("PicoJSON: No file found at: {}. Initializing empty _G.db", clean_path);
		g_game_state = picojson::value(picojson::object());

		lua_newtable(gLuaState);
		lua_setglobal(gLuaState, "db");
		return true;
	}

	std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	std::string err;
	picojson::parse(g_game_state, json_str.begin(), json_str.end(), &err);

	if (!err.empty()) {
		state_log.error("PicoJSON Parse Error: {}", err);
		return false;
	}

	state_log.info("--- PicoJSON successfully parsed data from {} ---", clean_path);
	dump_json_to_log(g_game_state);
	state_log.info("-------------------------------------------------------");

	picojson_to_lua(gLuaState, g_game_state);

	lua_setglobal(gLuaState, "db");

	state_log.info("C++ successfully injected state table into _G.db");
	return true;
}

void ck_state_save(const char* path) {
	if (path == nullptr) return;
	state_log.info("PicoJSON: C++ triggered ck_state_save for path: {}", path);
}
