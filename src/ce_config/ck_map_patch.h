#ifndef CK_MAP_PATCH_H
#define CK_MAP_PATCH_H

#include <string>

bool ck_map_patch_header(const std::string& mapFilePath, const std::string& mapName, int newIndex);
void ck_map_register_path(const std::string& mapFile, const std::string& fullPath);
const char* ck_map_resolve_path(const char* name);

#endif
