#include "ck_i18n.h"
#include "ck_encoding.h"

#include <unordered_map>
#include <fstream>
#include <string_view>
#include "picojson.h"

namespace ck::i18n {
    constexpr std::string_view LANG_DIR      = "../src/ck_i18n/languages/";
    constexpr std::string_view LANG_EXT      = ".json";
    constexpr std::string_view FALLBACK_LANG = "english";

    static std::unordered_map<std::string, std::string> g_translation_table;
    static std::string g_fallback_lang = "english";

    static void flatten_json(const picojson::value& val, const std::string& prefix) {
        if (val.is<picojson::object>()) {
            const auto& obj = val.get<picojson::object>();
            for (const auto& [key, child_val] : obj) {
                std::string new_prefix = prefix.empty() ? key : prefix + "." + key;
                flatten_json(child_val, new_prefix);
            }
        } else if (val.is<std::string>()) {
            std::string raw_utf8 = val.get<std::string>();
            g_translation_table[prefix] = utf8_to_cp1251(raw_utf8);
        }
    }

    void load_language(const std::string& language) {
        g_translation_table.clear();

        std::string path = std::string(LANG_DIR) + language + std::string(LANG_EXT);
        std::ifstream file(path);

        if (!file.is_open() && language != FALLBACK_LANG) {
            path = std::string(LANG_DIR) + std::string(FALLBACK_LANG) + std::string(LANG_EXT);
            file.open(path);
        }

        if (!file.is_open()) return;

        picojson::value root;
        std::string err = picojson::parse(root, file);
        if (!err.empty()) return;

        flatten_json(root, "");
    }

    const char* t(const std::string& key) {
        auto it = g_translation_table.find(key);
        if (it != g_translation_table.end()) {
            return it->second.c_str();
        }

        return key.c_str();
    }
}
