#include "ck_config_patch.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <regex>

namespace fallout {
    bool configSetString(Config*, const char*, const char*, const char*);
}

static std::vector<CkConfigPatch> gPatches;

static int ck_find_last_index(const std::string& filePath, const std::string& prefix) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::string upper = filePath;
        for (char& c : upper) c = toupper(c);
        file.open(upper);
    }
    if (!file.is_open()) {
        std::cerr << "[CK Config Patch] Cannot open for index scan: " << filePath << std::endl;
        return -1;
    }

    int lastIndex = -1;
    std::string line;
    std::string searchPrefix = "[" + prefix + " ";

    while (std::getline(file, line)) {
        if (line.find(searchPrefix) == 0) {
            size_t start = searchPrefix.size();
            size_t end   = line.find(']', start);
            if (end != std::string::npos) {
                try {
                    int idx = std::stoi(line.substr(start, end - start));
                    if (idx > lastIndex) lastIndex = idx;
                } catch (...) {}
            }
        }
    }

    return lastIndex;
}

int ck_config_next_map_index(const std::string& filePath) {
    int last = ck_find_last_index(filePath, "Map");
    std::cout << "[CK Config Patch] Last map index in " << filePath << ": " << last << std::endl;
    return last == -1 ? 0 : last + 1;
}

int ck_config_next_area_index(const std::string& filePath) {
    int last = ck_find_last_index(filePath, "Area");
    std::cout << "[CK Config Patch] Last area index in " << filePath << ": " << last << std::endl;
    return last == -1 ? 0 : last + 1;
}

void ck_config_patch_add(const std::string& filePath,
                         const std::string& section,
                         const std::string& key,
                         const std::string& value) {
    gPatches.push_back({ filePath, section, key, value });
    std::cout << "[CK Config Patch] Registered: ["
              << section << "] " << key << " = " << value
              << " (" << filePath << ")" << std::endl;
}

void ck_config_patch_apply(fallout::Config* config, const char* filePath) {
    if (config == nullptr || filePath == nullptr) return;

    int applied = 0;
    for (const auto& patch : gPatches) {
        // quick compare (no separation/registry)
        // "data\\city.txt" == "data/city.txt"
        std::string patchPath = patch.filePath;
        std::string checkPath = filePath;

        // normalize slashes
        for (char& c : patchPath) if (c == '\\') c = '/';
        for (char& c : checkPath) if (c == '\\') c = '/';

        if (patchPath == checkPath) {
            fallout::configSetString(config,
                patch.section.c_str(),
                patch.key.c_str(),
                patch.value.c_str());
            applied++;
        }
    }

    if (applied > 0) {
        std::cout << "[CK Config Patch] Applied " << applied
                  << " patches to: " << filePath << std::endl;
    }
}

void ck_config_patch_clear() {
    gPatches.clear();
    std::cout << "[CK Config Patch] Cleared all patches." << std::endl;
}
