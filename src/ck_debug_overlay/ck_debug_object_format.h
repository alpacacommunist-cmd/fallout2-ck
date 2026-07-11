#ifndef CK_DEBUG_OBJECT_FORMAT_H
#define CK_DEBUG_OBJECT_FORMAT_H

#include <string>

extern "C" { int ck_critter_get_max_hp(void* ptr); }

namespace fallout {
    struct Object;
	int critterGetStat(Object* critter, int stat);
}

namespace ck::debug {
    std::string format_object_data(fallout::Object* obj, int obj_type);
}

#endif
