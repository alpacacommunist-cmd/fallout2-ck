#include "ck_registry.h"
#include "ck_ids.h"
#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Registry [Modified]");

namespace {
    const std::string SYSTEM_MOD_ID = "__ck_system__";
}

namespace ck::registry::modified {
    int add(fallout::Object* obj, const LuaMeta& meta) {
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
        LuaMeta final_meta = meta;
        final_meta.source_sid = obj->sid;
        final_meta.source_pid = obj->pid;
        if (final_meta.mod_id.empty()) {
            const char* current_mod_id = ck_get_current_mod_id();
            final_meta.mod_id = current_mod_id != nullptr ? current_mod_id : SYSTEM_MOD_ID;
        }

        // write to registry
        g_modified_objects[lua_id] = CkModifiedObject{ obj, lua_id, final_meta };
        g_ptr_to_lua_id[obj] = lua_id;

		// assign sid
        obj->sid = ck::ids::make_sid_modified(obj, lua_id);

        log.debug("Registered modified object. LuaID: {}, Vanilla SID: {}, New Packed SID: {}",
                  lua_id, final_meta.source_sid, obj->sid);

        return lua_id;
    }

	void clear_for_mod(std::string_view mod_id) {
		int restored_sids_count = 0;

        auto mod_it = g_modified_objects.begin();
        while (mod_it != g_modified_objects.end()) {
            if (mod_it->second.meta.mod_id == mod_id) {
                if (mod_it->second.ptr != nullptr) {
                    mod_it->second.ptr->sid = mod_it->second.meta.source_sid;
                    g_ptr_to_lua_id.erase(mod_it->second.ptr);
                    restored_sids_count++;
                }
                mod_it = g_modified_objects.erase(mod_it);
            } else {
                ++mod_it;
            }
        }

        if (restored_sids_count > 0) {
            log.info("Hot Reload: Restored {} modified SIDs for mod '{}'", restored_sids_count, mod_id);
        }
	}

    void restore_original_sids() {
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

    void reapply_lua_sids() {
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
        if (it != g_modified_objects.end()) {
            return &it->second;
        }
        return nullptr;
    }
}

int ck_registry_modify_object(void* ptr) {
	if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);

	return ck::registry::modified::add(object);
}
