#pragma once

namespace fallout {
    struct Object;

    enum ObjectType : int;
    enum MapElevation : int;
}

namespace ck::utils::types {
    fallout::ObjectType object_type(fallout::Object* object);
    fallout::MapElevation map_elevation(int elevation);
}
