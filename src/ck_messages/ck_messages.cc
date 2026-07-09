#include "ck_messages/ck_messages.h"
#include "ck_log.h"
#include <unordered_map>
#include <algorithm>

static const Logger log("CK Messages");

namespace ck {

	static std::unordered_map<std::string, std::unordered_map<int, std::string>> g_strings;
	static std::unordered_map<fallout::MessageList*, std::string> g_list_registry;

	static std::string normalize_path(const std::string& path) {
		std::string result = path;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);

		for (char& c : result) if (c == '\\') c = '/';

		return result;
	}

	void messages_add_string(const std::string& msg_file, int msg_id, const std::string& text) {
		std::string file_key = normalize_path(msg_file);
		g_strings[file_key][msg_id] = text;
		log.debug("Registered string for {}: [{}] = {}", file_key, msg_id, text);
	}

	void messages_on_list_loaded(fallout::MessageList* list, const char* path) {
		if (list == nullptr || path == nullptr) return;

		std::string file_key = normalize_path(path);

		if (file_key.find("map.msg") != std::string::npos) {
			g_list_registry[list] = "game/map.msg";
		} else if (file_key.find("worldmap.msg") != std::string::npos) {
			g_list_registry[list] = "game/worldmap.msg";
		}
	}

	void messages_on_list_free(fallout::MessageList* list) {
		if (list != nullptr) g_list_registry.erase(list);
	}

	const char* messages_on_get_message(fallout::MessageList* list, int num, const char* default_text) {
		if (list == nullptr) return default_text;

		auto list_it = g_list_registry.find(list);
		if (list_it == g_list_registry.end()) return default_text;

		const std::string& file_key = list_it->second;

		auto file_it = g_strings.find(file_key);
		if (file_it != g_strings.end()) {
			auto string_it = file_it->second.find(num);
			if (string_it != file_it->second.end()) return string_it->second.c_str();
		}

		return default_text;
	}

	void messages_clear() {
		g_strings.clear();
		g_list_registry.clear();
		log.info("Runtime message registries cleared");
	}

}
