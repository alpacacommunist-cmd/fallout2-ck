#include "ck_ids.h"
#include "ck_registry/ck_registry.h"
#include "ck_mods/ck_mods.h"
#include "utils/fallout/types/object.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry_modified.cc"};

namespace ck_object = ck::utils::fallout::types::object;

namespace ck::registry::modified {
    static void free_object(fallout::Object* object) {
        if (object == nullptr) return;
        ck::ids::free_script_index(ck_object::script_index(object));
    }

    bool add(fallout::Object *object, const char *mod_id) {
        if (object == nullptr) return false;

        const RegistryEntry *existing_entry = ck::registry::get(object);
        if (existing_entry != nullptr) {
            logger.error("Can't bind to object: {}, entry already exists", ck_object::id(object));
            return false;
        }

        if (mod_id == nullptr) {
            const char *current_mod_id = ck::mods::current_mod_id();
            mod_id = (current_mod_id != nullptr) ? current_mod_id : ck::mods::system_mod_id();
        }

        RegistryEntry entry { .object = object, .mod_id = mod_id, .status = Status::Modified };
        g_object_registry[object] = std::move(entry);

        if (ck_object::script_index(object) != -1) {
            ck::ids::add_custom_script_index(ck_object::script_index(object), ck::ids::object_type(object), mod_id);
            logger.info.banner("tracking script_index: {}", ck_object::script_index(object));
        }

        logger.debug("Registered modified object. object_id: {}, script_index: {}",
                ck_object::id(object), ck_object::script_index(object));

        return true;
    }

    bool restore(fallout::Object *object) {
        if (object == nullptr) return false;

        auto it = g_object_registry.find(object);
        if (it != g_object_registry.end() && it->second.status == Status::Modified) {
            free_object(object);

            logger.debug("removing entry (object_id): {}", ck_object::id(object));
            g_object_registry.erase(it);

            return true;
        }

        return false;
    }

    void clear_for_mod(const char *mod_id) {
        if (mod_id == nullptr) return;

        for (auto it = g_object_registry.begin(); it != g_object_registry.end();) {
            const auto &entry = it->second;

            if (entry.status == Status::Modified && entry.mod_id == mod_id) {
                free_object(entry.object);
                logger.debug("removing entry (object_id: {}) (mod_id: {})",
                        entry.object ? ck_object::id(entry.object) : -1, mod_id);

                it = g_object_registry.erase(it);
            } else {
                ++it;
            }
        }
    }

    void free_script_indexes() {
        for (const auto &[object, entry] : g_object_registry) {
            if (entry.status == Status::Modified && entry.object != nullptr) {
                ck::ids::free_script_index(ck_object::script_index(entry.object));
            }
        }
    }
}

bool ck_registry_modify_object(fallout::Object* object) {
    return ck::registry::modified::add(object, nullptr);
}

bool ck_registry_object_is_modified(fallout::Object* object) {
    const auto* existing_entry = ck::registry::get(object);

    return existing_entry != nullptr && existing_entry->is_modified();
}

bool ck_registry_restore_modified_object(fallout::Object* object) {
    return ck::registry::modified::restore(object);
}
