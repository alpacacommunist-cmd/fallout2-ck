#include "ck_assets/assets_bindings.h"
#include "ck_assets/ck_asset_registry.h"

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

extern CkAssetRegistry gAssetRegistry;

namespace {

// ck.assets.resolve("temple_of_trials:scenery/tree10")
// returns table or nil if mod isn't registered
static int l_assets_resolve(lua_State* L) {
    const char* key = luaL_checkstring(L, 1);

    CkFrm* frm = ck_assets_resolve(gAssetRegistry, key);

    auto it = gAssetRegistry.assets.find(key);
    if (it == gAssetRegistry.assets.end()) {
        lua_pushnil(L);
        return 1;
    }

    const CkAsset& asset = it->second;

    lua_newtable(L);

    // key
    lua_pushstring(L, key);
    lua_setfield(L, -2, "key");

    // valid — .frm loaded or not
    lua_pushboolean(L, asset.frm.valid);
    lua_setfield(L, -2, "valid");

    // artId, fid, pid — -1 if not found in engine 
    lua_pushinteger(L, asset.artId);
    lua_setfield(L, -2, "artId");

    lua_pushinteger(L, asset.fid);
    lua_setfield(L, -2, "fid");

    lua_pushinteger(L, asset.pid);
    lua_setfield(L, -2, "pid");

	lua_pushinteger(L, asset.objectType);
	lua_setfield(L, -2, "objectType");

	lua_pushboolean(L, asset.objectType == fallout::OBJ_TYPE_TILE);
	lua_setfield(L, -2, "isTile");

    // lookupFailed — looked in engine and failed
    lua_pushboolean(L, asset.lookupFailed);
    lua_setfield(L, -2, "lookupFailed");

    // filePath — debug
    lua_pushstring(L, asset.filePath.c_str());
    lua_setfield(L, -2, "filePath");

    return 1;
}

// ck.assets.register("temple_of_trials", "mods/temple_of_trials/assets")
static int l_assets_register(lua_State* L) {
    const char* modId    = luaL_checkstring(L, 1);
    const char* basePath = luaL_checkstring(L, 2);

    ck_assets_register_mod(gAssetRegistry, modId, std::string("../") + basePath);
    return 0;
}

static const luaL_Reg assets_lib[] = {
    { "resolve",  l_assets_resolve },
    { "register", l_assets_register },
    { nullptr,    nullptr }
};

} // namespace

int luaopen_ck_assets(lua_State* L) {
    luaL_newlib(L, assets_lib);
    return 1;
}
