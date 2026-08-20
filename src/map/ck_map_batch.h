#ifndef CK_MAP_BATCH_H
#define CK_MAP_BATCH_H

#include "ck_api.h"

struct CkFFITile {
    int tile;
    int fid;
    int roof_block_id;
};

struct CkFFIScenery { int tile; int fid; };
struct CkFFIBlocker { int tile; int fid; };
struct CkFFIClear { int tile; };

CK_API void ck_map_batch_tiles(const CkFFITile* tiles, int count);
CK_API void ck_map_batch_roof_tiles(const CkFFITile* tiles, int count);

CK_API void ck_map_batch_scenery(const CkFFIScenery* sceneries, int count);

CK_API void ck_map_batch_blockers(const CkFFIBlocker* blockers, int count);
CK_API void ck_map_batch_clear(const CkFFIClear* tiles, int count);

#endif
