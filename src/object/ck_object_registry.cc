#include "ck_utils.h"
#include "ck_ids.h"
#include "object/ck_object_registry.h"
#include "object/ck_item.h"

#include "ck_log.h"
static const Logger log("CK Object Registry");

CkObjectRegistry gObjectRegistry;

const LuaMeta* CkObjectRegistry::get_meta(int lua_id) const {
    auto it = objects.find(lua_id);
    if (it == objects.end() || !it->second.alive) return nullptr;
    return &it->second.meta;
}

int CkObjectRegistry::add(fallout::Object* obj, const LuaMeta& meta) {
    int id = next_id++;
    objects[id] = { obj, id, true, meta };

    return id;
}

void CkObjectRegistry::destroy_objects_for_mod(const char* target_mod_id) {
	log.info("Hot Reload: Looking for {} objects", target_mod_id);

    if (target_mod_id == nullptr) return;
    std::string mod_id_str(target_mod_id);

    int count = 0;

    auto it = objects.begin();
    while (it != objects.end()) {
        if (it->second.alive && it->second.meta.mod_id == mod_id_str && it->second.ptr != nullptr) {
            // ck_scripting_on_object_destroyed gets triggered in engine (inside objectDestroy)
            // and calls remove_by_ptr.
			fallout::reg_anim_clear(it->second.ptr);
            fallout::objectDestroy(it->second.ptr, nullptr);

            it = objects.begin();
            count++;
        } else { ++it; }
    }

    if (count > 0) {
        log.info("Hot Reload: Physicaly destroyed {} objects belonging to mod '{}'", count, mod_id_str);
    }
}

int CkObjectRegistry::remove_by_ptr(fallout::Object* ptr) {
    if (ptr == nullptr) return false;

    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (it->second.alive && it->second.ptr == ptr) {
            it->second.alive = false;

            std::string obj_tag = it->second.meta.tag;
            int deleted_id      = it->second.lua_id;

			objects.erase(it);

			log.debug("Engine destroyed object [ID: {}, Tag: '{}']. Managed registry size: {}",
					deleted_id,
					obj_tag.empty() ? "mass_object (untagged)" : obj_tag,
					objects.size());

            return deleted_id;
        }
    }

    return -1;
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

void CkObjectRegistry::clear() {
    objects.clear();
    next_id = 1;

	log.info("Cleared objects, current registry size: 0");
}

void ck_registry_clear() {
	gObjectRegistry.clear();
}
