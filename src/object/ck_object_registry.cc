#include "ck_utils.h"
#include "ck_ids.h"
#include "object/ck_object_registry.h"
#include "object/ck_item.h"

#include "ck_log.h"
static const Logger log("CK Object Registry");

CkObjectRegistry gObjectRegistry;

const LuaCritterMeta* CkObjectRegistry::get_meta(int lua_id) const {
    auto it = objects.find(lua_id);
    if (it == objects.end() || !it->second.alive) return nullptr;
    return &it->second.meta;
}

int CkObjectRegistry::add(fallout::Object* obj, const LuaCritterMeta& meta) {
    int id = next_id++;
    objects[id] = { obj, id, true, meta };

    return id;
}

bool CkObjectRegistry::remove_by_ptr(fallout::Object* ptr) {
    if (ptr == nullptr) return false;

    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (it->second.alive && it->second.ptr == ptr) {
            // it->second.alive = false;
			objects.erase(it);

            return true;
        }
    }
    return false;
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

		ck::clear_inventory(managed.ptr);
        fallout::objectDestroy(managed.ptr, nullptr);

        managed.alive = false;
		managed.ptr = nullptr;
        count++;
    }

    objects.clear();
    next_id = 1;

	log.info("Destroyed {} managed objects", count);
	ck_call_lua_hook("ckOnObjectsDestroyed");
}

void CkObjectRegistry::clear() {
    objects.clear();
    next_id = 1;
}

