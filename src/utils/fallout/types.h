#pragma once

namespace fallout {
    struct Object;

    enum ObjectType : int;
}

namespace fallout {
    ObjectType object_type(Object* object);
}
