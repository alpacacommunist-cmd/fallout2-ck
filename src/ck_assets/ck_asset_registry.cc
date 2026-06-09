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

static int ck_assets_obj_type(const std::string& name) {
    if (name == "scenery")  return fallout::OBJ_TYPE_SCENERY;
    if (name == "items")    return fallout::OBJ_TYPE_ITEM;
    if (name == "critters") return fallout::OBJ_TYPE_CRITTER;
    if (name == "walls")    return fallout::OBJ_TYPE_WALL;
    if (name == "tiles")    return fallout::OBJ_TYPE_TILE;
    if (name == "misc")     return fallout::OBJ_TYPE_MISC;
    return -1;
}

void ck_assets_register_mod(CkAssetRegistry& reg, const std::string& modId, const std::string& basePath) {
    reg.modPaths[modId] = basePath;
    std::cout << "[CK Assets] Registered mod: " << modId << " -> " << basePath << std::endl;
}

CkFrm* ck_assets_resolve(CkAssetRegistry& reg, const std::string& key) {
    auto it = reg.assets.find(key);
    if (it != reg.assets.end()) {
        // nullptr means we already tried and failed
        if (!it->second.frm.valid) return nullptr;
        return &it->second.frm;
    }

    std::string modId, assetPath;
    if (!split_key(key, modId, assetPath)) {
        std::cerr << "[CK Assets] Bad key format: " << key << std::endl;
        return nullptr;
    }

    auto modIt = reg.modPaths.find(modId);
    if (modIt == reg.modPaths.end()) {
        std::cerr << "[CK Assets] Unknown mod: " << modId << std::endl;
        return nullptr;
    }

    std::string filePath = modIt->second + "/" + assetPath + ".frm";

    CkAsset asset;
    asset.modId    = modId;
    asset.filePath = filePath;
    asset.frm      = ck_frm_load(filePath);

	// assetPath = "scenery/tree10" -> type="scenery", name="tree10"
	asset.lookupDone = true;
	auto slash = assetPath.find('/');
	if (slash != std::string::npos) {
		std::string typeName = assetPath.substr(0, slash);
		std::string artName  = assetPath.substr(slash + 1);

		int objectType = ck_assets_obj_type(typeName);
		if (objectType != -1) {
			asset.objectType = objectType;

			int artId = fallout::artListIndex(objectType, artName.c_str());
			if (artId != -1) {
				asset.artId = artId;
				asset.fid   = fallout::buildFid(objectType, artId, 0, 0, 0);

				asset.pid   = (objectType << 24) | artId;

				std::cout << "[CK Assets] Found art: " << artId << " -> " << artName.c_str() << std::endl;
			} else {
				asset.lookupFailed = true;
			}
		}
	}

    // puts in registry anyways even if it fails
    // next resolve will quietly return nullptr
    reg.assets[key] = std::move(asset);

    if (!reg.assets[key].frm.valid) return nullptr;
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
