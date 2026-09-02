#include "ck_registry/ck_registry.h"
#include "ck_ids.h"
#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Registry [Modified]");

namespace ck::common {
    const char* system_mod_id();
    const char* current_mod_id();
}

namespace ck::registry::modified {
    int add(fallout::Object* obj, LuaMeta meta) {
        if (obj == nullptr) return -1;

        // return id if already exists
        auto ptr_it = g_ptr_to_lua_id.find(obj);
        if (ptr_it != g_ptr_to_lua_id.end()) {
            if (g_modified_objects.contains(ptr_it->second)) {
                return ptr_it->second;
            }
        }

        int lua_id = next_lua_id();

		// fill meta
        meta.source_sid = obj->sid;
        meta.source_pid = obj->pid;
        if (meta.mod_id.empty()) {
            const char* current_mod_id = ck::common::current_mod_id();
            std::string mod_id = current_mod_id != nullptr ? current_mod_id : ck::common::system_mod_id();

            meta.mod_id = mod_id;
        }

        // write to registry
        g_modified_objects[lua_id] = CkModifiedObject{ obj, lua_id, std::move(meta) };
        g_ptr_to_lua_id[obj] = lua_id;

		// assign sid
        obj->sid = ck::ids::make_sid_modified(obj, lua_id);

        log.debug("Registered modified object. LuaID: {}, Vanilla SID: {}, New Packed SID: {}",
                  lua_id, meta.source_sid, obj->sid);

        return lua_id;
    }

	int restore(fallout::Object* obj) {
        if (obj == nullptr) return false;

        auto ptr_it = g_ptr_to_lua_id.find(obj);
        if (ptr_it == g_ptr_to_lua_id.end()) return false;

        int lua_id = ptr_it->second;
		int source_sid;

        auto modified_it = g_modified_objects.find(lua_id);
        if (modified_it != g_modified_objects.end()) {
			source_sid = modified_it->second.meta.source_sid;
            obj->sid = modified_it->second.meta.source_sid;

            g_modified_objects.erase(modified_it);
        }

        g_ptr_to_lua_id.erase(ptr_it);

        log.debug("Restored individual object by pointer [LuaID: {}]. Remaining modified size: {}",
                  lua_id, g_modified_objects.size());

        return source_sid;
    }

	void clear_for_mod(std::string_view mod_id) {
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
            log.info("Hot Reload: Restored {} modified SIDs for mod '{}'", restored_sids_count, mod_id);
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
            log.info("Restored {} original SIDs before save/exit", restored_count);
        }
    }

    void reapply_sids() {
        int reapplied_count = 0;
        for (const auto& [id, entry] : g_modified_objects) {
            if (entry.ptr != nullptr) {
                // reapply modified sid
                entry.ptr->sid = ck::ids::make_sid_modified(entry.ptr, id);
                reapplied_count++;
            }
        }
        if (reapplied_count > 0) {
            log.info("Reapplied {} Lua SIDs after save", reapplied_count);
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

bool ck_object_is_modified(int lua_id) {
    return ck::registry::modified::get(lua_id) != nullptr;
}

int ck_registry_restore_modified_object(void* ptr) {
	if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);

	return ck::registry::modified::restore(object);
}

