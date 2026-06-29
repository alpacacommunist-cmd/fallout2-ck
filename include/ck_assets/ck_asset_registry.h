#ifndef CK_ASSET_REGISTRY_H
#define CK_ASSET_REGISTRY_H

#include "ck_assets/ck_frm.h"
#include "ck_api.h"
#include <string>
#include <unordered_map>

#include <art.h>

struct CkAsset {
    std::string modId;
    std::string filePath;
    CkFrm frm;

    int artId       = -1;
    int fid         = -1;
    int pid         = -1;
	int objectType  = -1;
    bool lookupDone = false;  // tried engine lookup
    bool lookupFailed = false; // tried and failed
};

struct CkAssetFFI {
	bool valid = false;

    int art_id         = -1;
    int fid            = -1;
    int pid            = -1;
	int object_type    = -1;
	bool is_tile       = false;
    bool lookup_failed = false;  // tried and failed
};


// register mod and resolve assets "mod_id:path/to/asset"
struct CkAssetRegistry {
    // "temple_of_trials" -> "../mods/temple_of_trials/assets"
    std::unordered_map<std::string, std::string> modPaths;

    // "temple_of_trials:scenery/tree10" -> CkAsset
    std::unordered_map<std::string, CkAsset> assets;
};

void ck_assets_register_mod(CkAssetRegistry& reg, const std::string& modId, const std::string& basePath);
CkFrm* ck_assets_resolve_frm(CkAssetRegistry& reg, const std::string& key);
void ck_assets_unload_mod(CkAssetRegistry& reg, const std::string& modId);
void ck_assets_clear(CkAssetRegistry& reg);

CK_API void ck_assets_register(const char* mod_id, const char* base_path);
CK_API CkAssetFFI ck_assets_resolve(const char* key);
CK_API const char* ck_asset_file_path(const char* key);

#endif
