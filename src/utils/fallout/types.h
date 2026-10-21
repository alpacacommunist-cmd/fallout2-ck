#pragma once

namespace fallout {
    struct Object;

    enum ObjectType : int;
    enum MapElevation : int;
}

namespace ck::utils::types {
    int object_type(fallout::Object* object);
    fallout::MapElevation map_elevation(int elevation);
}

namespace f_types = ck::utils::types; // NOLINT
