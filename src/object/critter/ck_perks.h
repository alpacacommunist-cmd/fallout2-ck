#ifndef CK_PERKS_H
#define CK_PERKS_H

#include "ck_api.h"

namespace fallout {
    struct Object;
	extern Object* gDude;

    enum Perk : int;
    int perkGetRank(Object* critter, Perk perk);
    int perkAddForce(Object* critter, Perk perk);
}

namespace ck::perks {
}

CK_API void ck_get_perks_metadata(void (*callback)(const char* name, int value));
CK_API int player_perk(int perk_id);
CK_API int player_perk_add_rank(int perk_id);

#endif
