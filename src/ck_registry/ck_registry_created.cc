#include "ck_registry/ck_registry.h"

#include "ck_log.h"
static const Logger log("CK Registry [Created]");

namespace ck::registry::created {
    int add(fallout::Object* obj, LuaMeta meta) {
        if (!obj) return -1;
        int lua_id = next_lua_id();

        g_created_objects[lua_id] = CkCreatedObject{ obj, lua_id, std::move(meta) };
        g_ptr_to_lua_id[obj] = lua_id;
        return lua_id;
    }

	void clear_for_mod(std::string_view mod_id) {
        std::vector<fallout::Object*> to_destroy;

        for (const auto& [id, managed] : g_created_objects) {
            if (managed.meta.mod_id == mod_id && managed.ptr != nullptr) {
                to_destroy.push_back(managed.ptr);
            }
        }

        for (fallout::Object* obj : to_destroy) {
            fallout::reg_anim_clear(obj);
            fallout::objectDestroy(obj, nullptr);
            // objectDestroy calls on_object_destroyed hook which clears g_ptr_to_lua_id
        }

        std::erase_if(g_created_objects, [mod_id](const auto& item) {
            return item.second.meta.mod_id == mod_id;
        });

        if (!to_destroy.empty()) {
            log.info("Hot Reload: Physically destroyed {} temporary objects for mod '{}'", to_destroy.size(), mod_id);
        }
    }

    int remove_by_ptr(fallout::Object* ptr) {
        if (ptr == nullptr) return -1;

        auto it = g_ptr_to_lua_id.find(ptr);
        if (it == g_ptr_to_lua_id.end()) {
			return -1;
		}

        int deleted_id = it->second;
        std::string obj_tag;

        auto created_it = g_created_objects.find(deleted_id);
        if (created_it != g_created_objects.end()) {
            obj_tag = created_it->second.meta.tag;
            g_created_objects.erase(created_it);
        }

        g_ptr_to_lua_id.erase(it);

        log.debug("Engine destroyed object [ID: {}, Tag: '{}']. Created registry size: {}",
                  deleted_id, obj_tag.empty() ? "untagged" : obj_tag, g_created_objects.size());

        return deleted_id;
    }

    const CkCreatedObject* get(int lua_id) {
        auto it = g_created_objects.find(lua_id);
        return (it == g_created_objects.end()) ? nullptr : &it->second;
    }

    fallout::Object* get_object(int lua_id) {
        auto it = g_created_objects.find(lua_id);
        return (it == g_created_objects.end()) ? nullptr : it->second.ptr;
    }
}
