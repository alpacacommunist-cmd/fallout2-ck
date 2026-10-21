#include "utils/fallout/types/proto.h"
#include "fallout2-ce/obj_types.h"

namespace ck::utils::types {
    int object_type(::fallout::Object* object) {
        return ck::utils::fallout::types::proto::type(object->pid);
    }

    ::fallout::MapElevation map_elevation(int elevation) {
        return static_cast<::fallout::MapElevation>(elevation);
    }
}
