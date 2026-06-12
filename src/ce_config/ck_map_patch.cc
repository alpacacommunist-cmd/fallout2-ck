#include "ck_map_patch.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>

// shift `index` MapHeader
// version(4) + name(16) + enteringTile(4) + enteringElevation(4)
// + enteringRotation(4) + localVariablesCount(4) + scriptIndex(4)
// + flags(4) + darkness(4) + globalVariablesCount(4) = 52

static const long NAME_OFFSET  = 4;
static const long NAME_SIZE    = 16;
static const long INDEX_OFFSET = 52;

bool ck_map_patch_header(const std::string& mapFilePath, const std::string& mapName, int newIndex) {
    FILE* f = fopen(mapFilePath.c_str(), "r+b");
    if (f == nullptr) {
        std::cerr << "[CK Map Patch] Cannot open: " << mapFilePath << std::endl;
        return false;
    }

    // name patch
    char nameBuf[NAME_SIZE] = {0};
    strncpy(nameBuf, mapName.c_str(), NAME_SIZE - 1);

    fseek(f, NAME_OFFSET, SEEK_SET);
    fwrite(nameBuf, 1, NAME_SIZE, f);

    // index patch just in case
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

    std::cout << "[CK Map Patch] Patched name=" << mapName
              << " index=" << newIndex
              << " in " << mapFilePath << std::endl;

    return true;
}
