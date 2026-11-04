#ifndef CK_TYPES_OBJECT_H
#define CK_TYPES_OBJECT_H

namespace fallout {
    struct Object;
}

namespace ck::utils::fallout::types {
    namespace object {
        int pid(::fallout::Object* object);
        int sid(::fallout::Object* object);
        int id(::fallout::Object* object);
        int script_index(::fallout::Object* object);

        void set_hidden(::fallout::Object* object);
        void unset_hidden(::fallout::Object* object);
    }
}

#endif
