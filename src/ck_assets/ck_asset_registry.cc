#include "ck_assets/ck_asset_registry.h"

extern CkAssetRegistry gAssetRegistry;

#include "ck_log.h"
static const Logger log("CK Assets");

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


CkFrm* ck_assets_resolve_frm(CkAssetRegistry& reg, const std::string& key) {
	std::string normalizedKey = key;
    for (char& c : normalizedKey) c = tolower(c);

    auto it = reg.assets.find(normalizedKey);
    if (it != reg.assets.end()) {
        // nullptr means we already tried and failed
        if (!it->second.frm.valid) return nullptr;
        return &it->second.frm;
    }

    std::string modId, assetPath;
    if (!split_key(normalizedKey, modId, assetPath)) {
        log.error("Bad key format: {}", normalizedKey);
        return nullptr;
    }

    auto modIt = reg.modPaths.find(modId);
    if (modIt == reg.modPaths.end()) {
        log.error("Unknown mod: {}", modId);
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

				log.info("Found art: {} -> {}", artId, artName.c_str());
			} else {
				asset.lookupFailed = true;
			}
		}
	}

    // puts in registry anyways even if it fails
    // next resolve will quietly return nullptr
    reg.assets[normalizedKey] = std::move(asset);

    if (!reg.assets[normalizedKey].frm.valid) return nullptr;
    return &reg.assets[normalizedKey].frm;
}

void ck_assets_register_mod(CkAssetRegistry& reg, const std::string& modId, const std::string& basePath) {
	std::string normalizedId = modId;
	for (char& c : normalizedId) c = tolower(c);

    reg.modPaths[normalizedId] = basePath;
    log.info("Registered mod: {} -> {}", normalizedId, basePath);
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
    log.info("Unloaded mod: {}", modId);
}

CkAssetFFI ck_assets_resolve_asset(CkAssetRegistry& reg, const char* key) {
    CkFrm* frm = ck_assets_resolve_frm(gAssetRegistry, key);

    auto it = gAssetRegistry.assets.find(key);
    if  (it == gAssetRegistry.assets.end()) return CkAssetFFI{};

    const CkAsset& asset = it->second;

	return { asset.frm.valid, asset.artId, asset.fid, asset.pid, asset.objectType,
		asset.objectType == fallout::OBJ_TYPE_TILE, asset.lookupFailed };
}

void ck_assets_clear(CkAssetRegistry& reg) {
    reg.assets.clear();
    reg.modPaths.clear();
}

// ffi
//

void ck_assets_register(const char* mod_id, const char* base_path) {
    ck_assets_register_mod(gAssetRegistry, mod_id, std::string("../") + base_path);
}

CkAssetFFI ck_assets_resolve(const char* key) {
	return ck_assets_resolve_asset(gAssetRegistry, key);
}

const char* ck_asset_file_path(const char* key) {
	std::string normalized_key = key;
    for (char& c : normalized_key) c = tolower(c);

    auto it = gAssetRegistry.assets.find(normalized_key);
    if (it != gAssetRegistry.assets.end()) return it->second.filePath.c_str();
    return nullptr;
}

