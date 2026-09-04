#include "ck_registry.h"

#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Registry");

namespace ck {
    namespace critter {
        void assign_script(fallout::Object* critter, int script_index, int lua_id);
    }

    namespace script {
        void remove_object_script(fallout::Object* object);
    }
}

namespace ck::registry {
    std::unordered_map<int, CkCreatedObject>  g_created_objects;
	std::unordered_map<int, CkModifiedObject> g_modified_objects;
    std::vector<CkDeletedObject>              g_deleted_objects;

    // Holds both CREATED and MODIFIED ptrs
    std::unordered_map<fallout::Object*, int> g_ptr_to_lua_id;

	static int g_next_id = 1;

	int  next_lua_id() { return g_next_id++; }
	void reset_lua_id_counter() { g_next_id = 1; }

	int find_by_ptr(fallout::Object* ptr) {
		if (ptr == nullptr) return -1;
		auto it = g_ptr_to_lua_id.find(ptr);
		return (it == g_ptr_to_lua_id.end()) ? -1 : it->second;
	}

	const LuaMeta* get_meta(int lua_id) {
        if (lua_id == -1) return nullptr;

		auto created_it = g_created_objects.find(lua_id);
		if (created_it != g_created_objects.end()) {
			return &created_it->second.meta;
		}

		auto modified_it = g_modified_objects.find(lua_id);
		if (modified_it != g_modified_objects.end()) {
			return &modified_it->second.meta;
		}

		return nullptr;
	}

	fallout::Object* get_object(int lua_id) {
		auto created_it = g_created_objects.find(lua_id);
		if (created_it != g_created_objects.end()) return created_it->second.ptr;

		auto modified_it = g_modified_objects.find(lua_id);
		if (modified_it != g_modified_objects.end()) return modified_it->second.ptr;

		return nullptr;
	}

    void clear_resources_for_mod(const char* mod_id) {
        if (mod_id == nullptr) return;

		std::string_view mod_str(mod_id);

		deleted::clear_for_mod(mod_str);
		modified::clear_for_mod(mod_str);
		created::clear_for_mod(mod_str);
    }

	void on_map_exit() {
		deleted::unhide();
		modified::restore_sids();

        // Clears [created], [modified], [deleted] registry object pointers
        ck::registry::clear();
	}

	void clear() {
        g_created_objects.clear();
        g_deleted_objects.clear();
		g_modified_objects.clear();

        g_ptr_to_lua_id.clear();
		reset_lua_id_counter();

        log.info("Cleared object registry entirely.");
	}
}

void ck_registry_clear() {
    ck::registry::clear();
}

void ck_registry_clear_for_mod(const char* target_mod_id) {
    ck::registry::clear_resources_for_mod(target_mod_id);
}

const char* ck_registry_object_get_mod_id(fallout::Object* object) {
    if (object == nullptr) return nullptr;

	const LuaMeta* meta = ck::registry::get_meta(ck::registry::find_by_ptr(object));
    return (meta != nullptr) ? meta->mod_id.data() : nullptr;
}

// MODIFIED critters don't have a tag. Only CREATED
const char* ck_registry_object_get_lua_tag(fallout::Object* object) {
    if (object == nullptr) return nullptr;

	const LuaMeta* meta = ck::registry::get_meta(ck::registry::find_by_ptr(object));
    return (meta != nullptr) ? meta->tag.data() : nullptr;
}

// Bind lua script to [CREATED] object
bool ck_registry_bind_to_object(int lua_id) {
    fallout::Object* object = ck::registry::created::get_object(lua_id);
    if (object == nullptr) return false;

    fallout::ObjectType type = fallout::objectTypeFromPid(object->pid);
    ck::script::remove_object_script(object);

    if (type == fallout::OBJ_TYPE_CRITTER) {
        ck::critter::assign_script(object, -1, lua_id);
    } else if (type == fallout::OBJ_TYPE_SCENERY) {
    }

    return true;
}

