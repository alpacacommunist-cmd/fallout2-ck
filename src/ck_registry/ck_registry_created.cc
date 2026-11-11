#include "ck_registry/ck_registry.h"
#include "utils/fallout/types/object.h"
#include "runtime/mods.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry_created.cc"};

namespace ck_object = ck::utils::fallout::types::object;

namespace fallout {
    // scripts.cc
    int scriptRemove(int index);
    // animation.cc
    int reg_anim_clear(Object* obj);
    // object.cc
    int objectDestroy(Object* object, Rect* rect);
}

namespace ck::registry::created {
    int add(fallout::Object *object, const char *tag, const char *mod_id) {
        if (!object) return -1;

        const RegistryEntry *existing_entry = ck::registry::get(object);
        if (existing_entry != nullptr) {
            return ck_object::id(object);
        }

        if (mod_id == nullptr) {
            const char *current_mod_id = ck::mods::current_mod_id();
            mod_id = (current_mod_id != nullptr) ? current_mod_id : ck::mods::system_mod_id();
        }

        RegistryEntry entry { .object = object, .mod_id = mod_id, .status = Status::Created };
        entry.tag = tag ? std::string(tag) : std::string();

        g_object_registry[object] = std::move(entry);

        return ck_object::id(object);
    }

    void clear_for_mod(const char *mod_id) {
        if (mod_id == nullptr) return;

        std::vector<fallout::Object *> to_destroy;
        for (const auto &[object, entry] : g_object_registry) {
            if (entry.is_created() && entry.mod_id == mod_id && object != nullptr) {
                logger.debug("destroying object [ID: {}, Tag: '{}']",
                        ck_object::id(entry.object), entry.tag.empty() ? "untagged" : entry.tag);

                to_destroy.push_back(object);
            }
        }

        for (fallout::Object *object : to_destroy) {
            // clears animation queue
            fallout::reg_anim_clear(object);
            // Removes script + active engine timers
            fallout::scriptRemove(ck_object::sid(object));
            // objectDestroy calls on_object_destroyed hook which clears g_ptr_to_lua_id
            fallout::objectDestroy(object, nullptr);
        }

        if (!to_destroy.empty()) {
            logger.info("Hot Reload: Physically destroyed {} temporary objects for mod '{}'", to_destroy.size(), mod_id);
        }
    }

    int remove_by_ptr(fallout::Object *object) {
        if (object == nullptr) return -1;

        auto it = g_object_registry.find(object);
        if (it == g_object_registry.end() || !it->second.is_created()) { return -1; }

        g_object_registry.erase(it);
        logger.debug("Created registry size: {}", g_object_registry.size());

        return 0;
    }
}

// FFI
bool ck_registry_object_is_created(fallout::Object* object) {
    const auto* existing_entry = ck::registry::get(object);

    return existing_entry != nullptr && existing_entry->is_created();
}

