#include "ck_ids.h"
#include "object/critter/ck_critter.h"
#include "ck_registry.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry.cc"};

namespace ck::registry {
    std::unordered_map<fallout::Object*, RegistryEntry> g_object_registry;

    std::unordered_map<int, CkCreatedObject>  g_created_objects;
	std::unordered_map<int, CkModifiedObject> g_modified_objects;

    // Holds both CREATED and MODIFIED ptrs
    std::unordered_map<fallout::Object*, int> g_ptr_to_lua_id;

	static int g_next_id = 1;

	int  next_lua_id() { return g_next_id++; }
	void reset_lua_id_counter() { g_next_id = 1; }

    const RegistryEntry *get(fallout::Object *object) {
        if (object == nullptr) return nullptr;

        auto reg_it = g_object_registry.find(object);
        if (reg_it == g_object_registry.end()) return nullptr;

        return &reg_it->second;
    }

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

		deleted::clear_for_mod(mod_id);
		modified::clear_for_mod(mod_id);
		created::clear_for_mod(mod_id);
    }

	void clear_context() {
        // Restores original object data
		deleted::unhide();
		modified::free_script_indexes();

        // Clears [created], [modified], [deleted] registry object pointers
        ck::registry::clear();
	}

	void clear() {
        g_created_objects.clear();
        g_object_registry.clear();
		g_modified_objects.clear();

        g_ptr_to_lua_id.clear();
		reset_lua_id_counter();

        logger.info("Cleared object registry entirely.");
	}
}

void ck_registry_clear() {
    ck::registry::clear();
}

const char* ck_registry_object_get_mod_id(fallout::Object* object) {
    const auto* existing_entry = ck::registry::get(object);
    if (existing_entry != nullptr) {
        return existing_entry->mod_id;
    }

    return nullptr;
}

const char* ck_registry_object_get_lua_tag(fallout::Object* object) {
    const auto* existing_entry = ck::registry::get(object);
    if (existing_entry != nullptr && existing_entry->is_created()) {
        return existing_entry->tag.data();
    }

    return nullptr;
}

// Bind lua script to [CREATED] object
bool ck_registry_bind_to_object(fallout::Object* object) {
    const auto* existing_entry = ck::registry::get(object);
    if (existing_entry == nullptr || !existing_entry->is_created()) return false;

    if (ck::ids::is_critter(object)) {
        ck::critter::assign_script(object, -1);
    } else if (ck::ids::is_scenery(object)) {
    }

    return true;
}

