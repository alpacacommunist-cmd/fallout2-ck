#include "ck_ids.h"
#include "ck_registry/ck_registry.h"
#include "ck_mods/ck_mods.h"

#include "fallout2-ce/obj_types.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry_modified.cc"};

namespace ck::registry::modified {
    static void free_object(fallout::Object* object) {
        ck::ids::free_script_index(object->scriptIndex);
        g_ptr_to_lua_id.erase(object);
    }

    int add(fallout::Object* object, LuaMeta meta) {
        if (object == nullptr) return -1;

        int lua_id = ck::registry::find_by_ptr(object);
        if ((lua_id) != -1) return lua_id;

        lua_id = next_lua_id();

		// fill meta
        meta.source_sid = object->sid;
        meta.source_pid = object->pid;
        if (!meta.mod_id) {
            const char* current_mod_id = ck::mods::current_mod_id();
            const char* mod_id = current_mod_id != nullptr ? current_mod_id : ck::mods::system_mod_id();

            meta.mod_id = mod_id;
        }

        // write to registry
        g_modified_objects[lua_id] = CkModifiedObject{ object, lua_id, std::move(meta) };
        g_ptr_to_lua_id[object] = lua_id;

        if (object->scriptIndex != -1) {
            ck::ids::add_custom_script_index(object->scriptIndex, ck::ids::object_type(object->pid), meta.mod_id);
            logger.info.banner("tracking script_index: {}", object->scriptIndex);
        }

        logger.debug("Registered modified object. lua_id: {}, script_index: {}", lua_id, object->scriptIndex);

        return lua_id;
    }

	bool restore(fallout::Object* object) {
        if (object == nullptr) return false;

        int lua_id = ck::registry::find_by_ptr(object);
        if (lua_id == -1) return false;

        auto it = g_modified_objects.find(lua_id);
        if (it != g_modified_objects.end()) {
            free_object(it->second.ptr);

            logger.debug("removing entry (lua_id): {}", lua_id);
            g_modified_objects.erase(it);

            return true;
        }

        return false;
    }

    void clear_for_mod(const char *mod_id) {
        std::erase_if(g_modified_objects, [mod_id](const auto &item) {
            if (item.second.meta.mod_id == mod_id) {
                free_object(item.second.ptr);
                logger.debug("removing entry (lua_id: {}) (mod_id: {})", item.second.lua_id, mod_id);

                return true;
            }

            return false;
        });
    }

    void free_script_indexes() {
        int restored_count = 0;
        for (const auto& [id, entry] : g_modified_objects) {
            if (entry.ptr != nullptr) {
                ck::ids::free_script_index(entry.ptr->scriptIndex);
            }
        }
    }

    const CkModifiedObject* get(int lua_id) {
        auto it = g_modified_objects.find(lua_id);
        return (it != g_modified_objects.end()) ? &it->second : nullptr;
    }
}

int ck_registry_modify_object(void* ptr) {
	if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);

	return ck::registry::modified::add(object);
}

bool ck_registry_object_is_modified(int lua_id) {
    return ck::registry::modified::get(lua_id) != nullptr;
}

bool ck_registry_restore_modified_object(void* ptr) {
	if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);

	return ck::registry::modified::restore(object);
}

