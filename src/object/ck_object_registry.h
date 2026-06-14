#ifndef CK_OBJECT_REGISTRY_H
#define CK_OBJECT_REGISTRY_H

#include <unordered_map>

namespace fallout {
    struct Object;
}

struct CkManagedObject {
    fallout::Object* ptr   = nullptr;
    int              luaId = -1;
    bool             alive = true;
};

class CkObjectRegistry {
public:
    int  add(fallout::Object* obj);

    fallout::Object* get(int luaId) const;
    int find_by_ptr(fallout::Object* ptr) const;

    void destroy_all();
    void clear();

private:
    int next_id = 1;
    std::unordered_map<int, CkManagedObject> objects;
};

extern CkObjectRegistry gObjectRegistry;

#endif
