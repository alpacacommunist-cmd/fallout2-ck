#include <cstddef>
#include "fallout2-ce/obj_types.h"
#include "fallout2-ce/animation_defs.h"

void validate_object_structure() {
    using namespace fallout;
    static_assert(sizeof(Object) == 160, "🚨 fallout::Object size changed, refresh lua header!");

    static_assert(offsetof(Object, data) == 48, "🚨 fields.data offsets missmatch!");
    static_assert(offsetof(Object, pid) == 120, "🚨 fields.pid offsets missmatch!");
    static_assert(offsetof(Object, owner) == 144, "🚨 fields.owner offsets missmatch!");
}

void validate_animation_enums() {
    using namespace fallout;

    static_assert(ANIMATION_REQUEST_NONE == 0x00, "🚨 ANIMATION_REQUEST_NONE mismatch!");
    static_assert(ANIMATION_REQUEST_UNRESERVED == 0x01, "🚨 ANIMATION_REQUEST_UNRESERVED mismatch!");
    static_assert(ANIMATION_REQUEST_RESERVED == 0x02, "🚨 ANIMATION_REQUEST_RESERVED mismatch!");
    static_assert(ANIMATION_REQUEST_NO_STAND == 0x04, "🚨 ANIMATION_REQUEST_NO_STAND mismatch!");
    static_assert(ANIMATION_REQUEST_PING == 0x100, "🚨 ANIMATION_REQUEST_PING mismatch!");
    static_assert(ANIMATION_REQUEST_INSIGNIFICANT == 0x200, "🚨 ANIMATION_REQUEST_INSIGNIFICANT mismatch!");
}
