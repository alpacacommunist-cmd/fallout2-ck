#ifndef CK_ASSET_REGISTRY_H
#define CK_ASSET_REGISTRY_H

#include "ck_assets/ck_frm.h"
#include <string>
#include <unordered_map>

struct CkAsset {
    std::string modId;
    std::string filePath;
    CkFrm frm;            // lazy loads on first resolve
};

// register mod and resolve assets "mod_id:path/to/asset"
struct CkAssetRegistry {
    // "temple_of_trials" -> "../mods/temple_of_trials/assets"
    std::unordered_map<std::string, std::string> modPaths;

    // "temple_of_trials:scenery/tree10" -> CkAsset
    std::unordered_map<std::string, CkAsset> assets;
};

void ck_assets_register_mod(CkAssetRegistry& reg, const std::string& modId, const std::string& basePath);
CkFrm* ck_assets_resolve(CkAssetRegistry& reg, const std::string& key);
void ck_assets_unload_mod(CkAssetRegistry& reg, const std::string& modId);
void ck_assets_clear(CkAssetRegistry& reg);

#endif
