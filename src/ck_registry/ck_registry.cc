#include "ck_ids.h"
#include "object/critter/ck_critter.h"
#include "ck_registry.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry.cc"};

namespace ck::registry {
    std::unordered_map<fallout::Object*, RegistryEntry> g_object_registry;

    const RegistryEntry *get(fallout::Object *object) {
        if (object == nullptr) return nullptr;

        auto reg_it = g_object_registry.find(object);
        if (reg_it == g_object_registry.end()) return nullptr;

        return &reg_it->second;
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
        g_object_registry.clear();
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

