#include "ck_map_registry.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool CkMapRegistry::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[CK Registry] No existing registry at " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // search: "key": { "mapIdx": N, "areaIdx": M }
        size_t keyStart = line.find('"');
        if (keyStart == std::string::npos) continue;
        size_t keyEnd = line.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) continue;

        std::string key = line.substr(keyStart + 1, keyEnd - keyStart - 1);
        if (key == "temple_of_trials" || line.find(':') == std::string::npos) continue;

        size_t mapPos = line.find("\"mapIdx\":");
        size_t areaPos = line.find("\"areaIdx\":");
        if (mapPos == std::string::npos || areaPos == std::string::npos) continue;

        int mapIdx  = std::stoi(line.substr(mapPos + 9));
        int areaIdx = std::stoi(line.substr(areaPos + 10));

        entries[key] = { mapIdx, areaIdx };
    }

    std::cout << "[CK Registry] Loaded " << entries.size() << " entries from " << path << std::endl;
    return true;
}

bool CkMapRegistry::save(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[CK Registry] Failed to write " << path << std::endl;
        return false;
    }

    file << "{\n";
    bool first = true;
    for (const auto& [key, entry] : entries) {
        if (!first) file << ",\n";
        file << "  \"" << key << "\": { \"mapIdx\": " << entry.mapIdx
             << ", \"areaIdx\": " << entry.areaIdx << " }";
        first = false;
    }
    file << "\n}\n";

    std::cout << "[CK Registry] Saved " << entries.size() << " entries to " << path << std::endl;
    return true;
}

CkMapRegistryEntry& CkMapRegistry::resolve(const std::string& key, int nextMapIdx, int nextAreaIdx) {
    auto it = entries.find(key);
    if (it != entries.end()) {
        return it->second;
    }

    std::cout << "[CK Registry] New entry: " << key
              << " -> mapIdx=" << nextMapIdx << " areaIdx=" << nextAreaIdx << std::endl;

    entries[key] = { nextMapIdx, nextAreaIdx };
    return entries[key];
}

bool CkMapRegistry::has(const std::string& key) const {
    return entries.find(key) != entries.end();
}
