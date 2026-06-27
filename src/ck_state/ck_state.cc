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

			else if (value.is<double>()) state_log.info("{}{}: {} (number)", spaces, key, value.get<double>());
			else if (value.is<std::string>()) state_log.info("{}{}: \"{}\"", spaces, key, value.get<std::string>());
			else if (value.is<bool>()) state_log.info("{}{}: {}", spaces, key, value.get<bool>() ? "true" : "false");
		}
	}
}

bool is_number(const std::string& s) {
	return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void picojson_to_lua(lua_State* L, const picojson::value& val) {
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

picojson::value lua_to_picojson(lua_State* L, int idx) {
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

bool ck_state_load(const char* path) {
	if (path == nullptr || gLuaState == nullptr) return false;

	std::string clean_path(path);
	for (char& c : clean_path) if (c == '\\') c = '/';

	std::ifstream file(clean_path);
	if (!file.is_open()) {
		state_log.warn("PicoJSON: No file found at: {}. Initializing empty _G.db", clean_path);
		g_game_state = picojson::value(picojson::object());

		return true;
	}

	std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	std::string err;
	picojson::parse(g_game_state, json_str.begin(), json_str.end(), &err);
	if (!err.empty()) return false;

	picojson_to_lua(gLuaState, g_game_state);

	lua_getglobal(gLuaState, "ck_state_sync_load");

	if (lua_isfunction(gLuaState, -1)) {
		lua_insert(gLuaState, -2);

		if (lua_pcall(gLuaState, 1, 0, 0) != LUA_OK) {
			state_log.error("Lua Load Hook Error: {}", lua_tostring(gLuaState, -1));
			lua_pop(gLuaState, 1);
		}
	} else {
		lua_pop(gLuaState, 2);
	}

	return true;
}

void ck_state_save(const char* path) {
	if (path == nullptr || gLuaState == nullptr) return;

	std::string clean_path(path);
	for (char& c : clean_path) if (c == '\\') c = '/';

	lua_getglobal(gLuaState, "ck_state_sync_save");

	if (!lua_isfunction(gLuaState, -1)) {
		lua_pop(gLuaState, 1);
		return;
	}

	if (lua_pcall(gLuaState, 0, 1, 0) != LUA_OK) {
		state_log.error("Lua Sync Hook Error: {}", lua_tostring(gLuaState, -1));
		lua_pop(gLuaState, 1);
		return;
	}

	if (!lua_istable(gLuaState, -1)) {
		state_log.error("PicoJSON Save: Lua hook did not return a valid state table!");
		lua_pop(gLuaState, 1);
		return;
	}

	picojson::value save_data = lua_to_picojson(gLuaState, -1);
	lua_pop(gLuaState, 1);

	std::ofstream file(clean_path);
	if (file.is_open()) {
		file << save_data.serialize(true);
		file.close();
		state_log.info("Game state successfully saved to: {}", clean_path);
	}
}
