#ifndef CK_OBJECT_REGISTRY_H
#define CK_OBJECT_REGISTRY_H

#include <unordered_map>
#include <string>

namespace fallout {
    struct Object;
}

struct LuaCritterMeta {
	int proto_sid;
};

struct CkManagedObject {
    fallout::Object* ptr   = nullptr;
    int              luaId = -1;
    bool             alive = true;

	LuaCritterMeta   meta;
};

class CkObjectRegistry {
public:
    int  add(fallout::Object* obj, const LuaCritterMeta& meta = {});
	bool remove_by_ptr(fallout::Object* ptr);

	const CkManagedObject* get_managed(int luaId) const;

    fallout::Object* get(int lua_id) const;
    int find_by_ptr(fallout::Object* ptr) const;
	const LuaCritterMeta* get_meta(int lua_id) const;

    void destroy_all();
    void clear();

private:
    int next_id = 1;
    std::unordered_map<int, CkManagedObject> objects;
};

extern CkObjectRegistry gObjectRegistry;

#endif
