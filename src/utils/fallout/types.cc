#include "proto_types.h"

namespace fallout {
    ObjectType object_type(Object* object) {
        return ProtoId(object).objectType();
    }
}
