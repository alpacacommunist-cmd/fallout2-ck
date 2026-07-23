#include "ck_registry/ck_registry.h"

#include "ck_log.h"
static const Logger log("CK Registry [Created]");

namespace ck::registry::created {

    int add(fallout::Object* obj, const LuaMeta& meta) {
        if (!obj) return -1;
        int id = next_lua_id();

        g_created_objects[id] = CkCreatedObject{ obj, id, meta };
        g_ptr_to_lua_id[obj] = id;
        return id;
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

    int find_by_ptr(fallout::Object* ptr) {
        auto it = g_ptr_to_lua_id.find(ptr);
        return (it == g_ptr_to_lua_id.end()) ? -1 : it->second;
    }

    const LuaMeta* get_meta(int lua_id) {
        auto it = g_created_objects.find(lua_id);
        return (it == g_created_objects.end()) ? nullptr : &it->second.meta;
    }
}
