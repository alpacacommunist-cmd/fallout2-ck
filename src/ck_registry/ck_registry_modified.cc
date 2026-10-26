#include "ck_registry/ck_registry.h"
#include "ck_mods/ck_mods.h"
#include "ck_ids.h"

#include "fallout2-ce/obj_types.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry_modified.cc"};

namespace ck::registry::modified {
    int add(fallout::Object* object, LuaMeta meta) {
        if (object == nullptr) return -1;

        // return id if already exists
        auto ptr_it = g_ptr_to_lua_id.find(object);
        if (ptr_it != g_ptr_to_lua_id.end()) {
            if (g_modified_objects.contains(ptr_it->second)) {
                return ptr_it->second;
            }
        }

        int lua_id = next_lua_id();

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

		// assign sid
        int custom_script_index = ck::ids::next_available_script_index(meta.mod_id);
        if (custom_script_index == -1) {
            logger.error("Couldn't allocate script_index!");
            return -1;
        }

        object->sid = ck::ids::make_packed_sid(object, custom_script_index);
        meta.custom_sid = object->sid;

        if (ck::ids::is_ck_sid(object->sid)) {
            logger.error("CK SID!");
        } else {
            logger.error("not CK SID!");
        }

        sid_to_lua_id[object->sid] = lua_id;
        logger.debug("Registered modified object. LuaID: {}, Vanilla SID: {}, New Packed SID: {}",
                  lua_id, meta.source_sid, object->sid);

        return lua_id;
    }

	int restore(fallout::Object* object) {
        if (object == nullptr) return false;

        auto ptr_it = g_ptr_to_lua_id.find(object);
        if (ptr_it == g_ptr_to_lua_id.end()) return false;

        int lua_id = ptr_it->second;
		int source_sid;

        auto modified_it = g_modified_objects.find(lua_id);
        if (modified_it != g_modified_objects.end()) {
			source_sid = modified_it->second.meta.source_sid;
            object->sid = modified_it->second.meta.source_sid;

            g_modified_objects.erase(modified_it);
        }

        g_ptr_to_lua_id.erase(ptr_it);

        logger.debug("Restored individual object by pointer [LuaID: {}]. Remaining modified size: {}",
                  lua_id, g_modified_objects.size());

        return source_sid;
    }

	void clear_for_mod(const char* mod_id) {
		int restored_sids_count = 0;

        auto modified_it = g_modified_objects.begin();
        while (modified_it != g_modified_objects.end()) {
            if (modified_it->second.meta.mod_id == mod_id) {
                if (modified_it->second.ptr != nullptr) {
                    modified_it->second.ptr->sid = modified_it->second.meta.source_sid;
                    g_ptr_to_lua_id.erase(modified_it->second.ptr);
                    restored_sids_count++;
                }
                modified_it = g_modified_objects.erase(modified_it);
            } else {
                ++modified_it;
            }
        }

        if (restored_sids_count > 0) {
            logger.info("Hot Reload: Restored {} modified SIDs for mod '{}'", restored_sids_count, mod_id);
        }
	}

    void restore_sids() {
        int restored_count = 0;
        for (const auto& [id, entry] : g_modified_objects) {
            if (entry.ptr != nullptr) {
                entry.ptr->sid = entry.meta.source_sid; // return source_sid
                restored_count++;
            }
        }
        if (restored_count > 0) {
            logger.info("Restored {} original SIDs before save/exit", restored_count);
        }
    }

    void reapply_sids() {
        int reapplied_count = 0;
        for (const auto& [id, entry] : g_modified_objects) {
            if (entry.ptr != nullptr) {
                // reapply modified sid
                entry.ptr->sid = entry.meta.custom_sid;
                reapplied_count++;
            }
        }
        if (reapplied_count > 0) {
            logger.info("Reapplied {} Lua SIDs after save", reapplied_count);
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

int ck_registry_restore_modified_object(void* ptr) {
	if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);

	return ck::registry::modified::restore(object);
}

