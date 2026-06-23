#include "ck_map_patch.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <algorithm>

#include "ck_log.h"
static const Logger log("CK Map Patch");

static const long NAME_OFFSET  = 4;
static const long NAME_SIZE    = 16;
static const long INDEX_OFFSET = 52;

bool ck_map_patch_header(const std::string& mapFilePath, const std::string& mapName, int newIndex) {
    FILE* f = fopen(mapFilePath.c_str(), "r+b");
    if (f == nullptr) {
		log.error("Cannot open map file: {}", mapFilePath);
        return false;
    }

    // name patch
    char nameBuf[NAME_SIZE] = {0};
    strncpy(nameBuf, mapName.c_str(), NAME_SIZE - 1);

    fseek(f, NAME_OFFSET, SEEK_SET);
    fwrite(nameBuf, 1, NAME_SIZE, f);

    // index patch
    fseek(f, INDEX_OFFSET, SEEK_SET);
    uint32_t value = (uint32_t)newIndex;
    unsigned char bytes[4] = {
        (unsigned char)((value >> 24) & 0xFF),
        (unsigned char)((value >> 16) & 0xFF),
        (unsigned char)((value >> 8)  & 0xFF),
        (unsigned char)(value & 0xFF)
    };
    fwrite(bytes, 1, 4, f);
    fclose(f);

	log.info("Patched name={} index={} in {}", mapName, newIndex, mapFilePath);

    return true;
}

static std::unordered_map<std::string, std::string> gMapPaths;

void ck_map_register_path(const std::string& mapFile, const std::string& fullPath) {
    // always lowercase in gMapPaths
    std::string lowerKey = mapFile;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

    gMapPaths[lowerKey] = fullPath;
	log.info("Registered path: {} -> {}", lowerKey, fullPath);
}

const char* ck_map_resolve_path(const char* name) {
    if (name == nullptr) return nullptr;
    static char path[512];

    std::string fullName = name;
    size_t dot = fullName.find('.');

    std::string key = (dot != std::string::npos) ? fullName.substr(0, dot) : fullName;
    std::string ext = (dot != std::string::npos) ? fullName.substr(dot) : "";

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

    // extension exists and isn't ".MAP" means engine looks for edg, sav etc
    // return nullptr and let engine find it on it's own
    if (!ext.empty() && ext != ".MAP") {
        return nullptr;
    }

    auto it = gMapPaths.find(key);
    if (it == gMapPaths.end()) {
        return nullptr;
    }

    strncpy(path, it->second.c_str(), sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    return path;
}
