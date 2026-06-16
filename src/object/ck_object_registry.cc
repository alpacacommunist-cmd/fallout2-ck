#include "ck_ids.h"
#include "object/ck_object_registry.h"
#include "object.h"
#include <iostream>

CkObjectRegistry gObjectRegistry;

const LuaCritterMeta* CkObjectRegistry::get_meta(int lua_id) const {
    auto it = objects.find(lua_id);
    if (it == objects.end() || !it->second.alive) return nullptr;
    return &it->second.meta;
}

int CkObjectRegistry::add(fallout::Object* obj, const LuaCritterMeta& meta) {
    int id = next_id++;
    objects[id] = { obj, id, true, meta };

	// std::cout << "[CK] Registering CRITTER META: name: " << meta.name << " description: " << meta.description
	// 	<< std::endl;
	//
	// std::cout << "[CK] Registering CRITTER META: sid: " << ck::clean_sid(obj->sid)
	// 	<< std::endl;

    return id;
}

const CkManagedObject* CkObjectRegistry::get_managed(int lua_id) const {
    auto it = objects.find(lua_id);
    if (it == objects.end() || !it->second.alive) return nullptr;
    return &it->second;
}

fallout::Object* CkObjectRegistry::get(int lua_id) const {
    auto it = objects.find(lua_id);
    if (it == objects.end() || !it->second.alive) return nullptr;
    return it->second.ptr;
}

int CkObjectRegistry::find_by_ptr(fallout::Object* ptr) const {
    for (const auto& [id, managed] : objects) {
        if (managed.alive && managed.ptr == ptr) return id;
    }
    return -1;
}

void CkObjectRegistry::destroy_all() {
    int count = 0;
    for (auto& [id, managed] : objects) {
        if (managed.ptr == nullptr || !managed.alive) continue;

        fallout::objectDestroy(managed.ptr, nullptr);

        managed.alive = false;
        count++;
    }

    objects.clear();
    next_id = 1;
    std::cout << "[CK Object Registry] Destroyed " << count << " managed objects" << std::endl;
}

void CkObjectRegistry::clear() {
    objects.clear();
    next_id = 1;
}
