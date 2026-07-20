// src/map/ck_map_batch.h
#ifndef CK_SCRIPTING_MAP_BATCH_H
#define CK_SCRIPTING_MAP_BATCH_H

#include "ck_api.h"

struct CkFFITile {
    int tile;
    int fid; // -1 means use key
    const char* key; // nullptr if fid != -1
};

struct CkFFIScenery { int tile; int fid; const char* key; };
struct CkFFIBlocker { int tile; int fid; };
struct CkFFIClear { int tile; };

CK_API void ck_map_batch_tiles(const CkFFITile* tiles, int count);
CK_API void ck_map_batch_scenery(const CkFFIScenery* sceneries, int count);
CK_API void ck_map_batch_blockers(const CkFFIBlocker* blockers, int count);
CK_API void ck_map_batch_clear(const CkFFIClear* tiles, int count);

#endif
