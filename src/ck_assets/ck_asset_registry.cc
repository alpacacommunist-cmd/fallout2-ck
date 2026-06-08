#include "ck_assets/ck_asset_registry.h"
#include <iostream>

// breaks "temple_of_trials:scenery/tree10" -> { "temple_of_trials", "scenery/tree10" }
// returns false if format's wrong
static bool split_key(const std::string& key, std::string& outModId, std::string& outPath) {
    auto pos = key.find(':');
    if (pos == std::string::npos) return false;

    outModId = key.substr(0, pos);
    outPath  = key.substr(pos + 1);
    return true;
}

void ck_assets_register_mod(CkAssetRegistry& reg, const std::string& modId, const std::string& basePath) {
    reg.modPaths[modId] = basePath;
    std::cout << "[CK Assets] Registered mod: " << modId << " -> " << basePath << std::endl;
}

CkFrm* ck_assets_resolve(CkAssetRegistry& reg, const std::string& key) {
    // already loaded
    auto it = reg.assets.find(key);
    if (it != reg.assets.end()) {
        return &it->second.frm;
    }

    // parse the key
    std::string modId, assetPath;
    if (!split_key(key, modId, assetPath)) {
        std::cerr << "[CK Assets] Bad key format: " << key << std::endl;
        return nullptr;
    }

    // known mod?
    auto modIt = reg.modPaths.find(modId);
    if (modIt == reg.modPaths.end()) {
        std::cerr << "[CK Assets] Unknown mod: " << modId << std::endl;
        return nullptr;
    }

    // make path and load
    std::string filePath = modIt->second + "/" + assetPath + ".frm";

    CkAsset asset;
    asset.modId    = modId;
    asset.filePath = filePath;
    asset.frm      = ck_frm_load(filePath);

    if (!asset.frm.valid) {
        std::cerr << "[CK Assets] Failed to load: " << filePath << std::endl;
        return nullptr;
    }

    reg.assets[key] = std::move(asset);
    return &reg.assets[key].frm;
}

void ck_assets_unload_mod(CkAssetRegistry& reg, const std::string& modId) {
    // clear mods assets
    auto it = reg.assets.begin();
    while (it != reg.assets.end()) {
        if (it->second.modId == modId) {
            it = reg.assets.erase(it);
        } else {
            ++it;
        }
    }

    reg.modPaths.erase(modId);
    std::cout << "[CK Assets] Unloaded mod: " << modId << std::endl;
}

void ck_assets_clear(CkAssetRegistry& reg) {
    reg.assets.clear();
    reg.modPaths.clear();
}
