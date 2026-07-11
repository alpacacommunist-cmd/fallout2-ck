#ifndef CK_DEBUG_OBJECT_FORMAT_H
#define CK_DEBUG_OBJECT_FORMAT_H

#include <string>

namespace fallout {
    struct Object;
}

namespace ck::debug {
    std::string format_object_data(fallout::Object* obj, int obj_type);
}

#endif
