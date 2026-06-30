#include "picojson.h"
#include "ck_state/ck_state.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

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

bool ck_state_load(const char* path) {
	if (path == nullptr || !ck::proxy::is_ready()) return false;

	std::string clean_path(path);
	for (char& c : clean_path) if (c == '\\') c = '/';

	std::ifstream file(clean_path);
	if (!file.is_open()) {
		state_log.warn("PicoJSON: No file found at: {}. Initializing empty state", clean_path);
		g_game_state = picojson::value(picojson::object());

        return ck::proxy::sync_state_load(g_game_state);
	}

	std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	std::string err;
	picojson::parse(g_game_state, json_str.begin(), json_str.end(), &err);
	if (!err.empty()) return false;

    return ck::proxy::sync_state_load(g_game_state);
}

void ck_state_save(const char* path) {
	if (path == nullptr || !ck::proxy::is_ready()) return;

	std::string clean_path(path);
	for (char& c : clean_path) if (c == '\\') c = '/';

	picojson::value save_data = ck::proxy::sync_state_save();

	std::ofstream file(clean_path);
	if (file.is_open()) {
		file << save_data.serialize(true);
		file.close();
		state_log.info("Game state successfully saved to: {}", clean_path);
	}
}
