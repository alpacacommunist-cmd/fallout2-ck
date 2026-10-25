#include "fallout2-ce/obj_types.h"

namespace ck::utils::fallout::types::object {
    int pid(::fallout::Object* object) {
        return object ? object->pid : -1;
    }
}
