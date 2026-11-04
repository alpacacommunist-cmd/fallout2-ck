#include "fallout2-ce/obj_types.h"

namespace ck::utils::fallout::types::object {
    int pid(::fallout::Object* object) {
        return object->pid;
    }

    int sid(::fallout::Object* object) {
        return object->sid;
    }

    int id(::fallout::Object* object) {
        return object->id;
    }

    int script_index(::fallout::Object* object) {
        return object->scriptIndex;
    }

    void set_hidden(::fallout::Object* object) {
		object->flags |= ::fallout::OBJECT_HIDDEN;
    }

    void unset_hidden(::fallout::Object* object) {
        object->flags &= ~::fallout::OBJECT_HIDDEN;
    }
}
