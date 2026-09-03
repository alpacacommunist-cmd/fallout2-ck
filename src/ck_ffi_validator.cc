#include <cstddef>
#include "obj_types.h"

void validate_object_structure() {
    static_assert(sizeof(fallout::Object) == 160, "🚨 fallout::Object size changed, refresh lua header!");
    
    static_assert(offsetof(fallout::Object, data) == 48, "🚨 fields.data offsets missmatch!");
    static_assert(offsetof(fallout::Object, pid) == 120, "🚨 fields.pid offsets missmatch!");
    static_assert(offsetof(fallout::Object, owner) == 144, "🚨 fields.owner offsets missmatch!");
}
