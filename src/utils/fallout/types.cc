#include "fallout2-ce/proto_types.h"

namespace ck::utils::types {
    using namespace fallout;

    ObjectType object_type(Object* object) {
        return ProtoId(object).objectType();
    }

    MapElevation map_elevation(int elevation) {
        return static_cast<MapElevation>(elevation);
    }
}
