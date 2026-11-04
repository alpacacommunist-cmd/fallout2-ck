#include "ck_registry.h"
#include "ck_mods/ck_mods.h"
#include "utils/fallout/types/object.h"

#include "log.h"
static constexpr logging::Context logger{"ck_registry_deleted.cc"};

namespace ck_object = ck::utils::fallout::types::object;

namespace ck::registry::deleted {
    void add(fallout::Object* object) {
        if (object == nullptr) return;

        const RegistryEntry* existing_entry = ck::registry::get(object);
        if (existing_entry != nullptr) return;

		const char* mod_id = ck::mods::current_mod_id();
		if (mod_id == nullptr) mod_id = ck::mods::system_mod_id();

        ck_object::set_hidden(object);
        RegistryEntry entry { .object = object, .mod_id = mod_id, .status = Status::Deleted };

        g_object_registry[object] = std::move(entry);
    }

    void remove(fallout::Object* object) {
        if (object == nullptr) return;

        const RegistryEntry* existing_entry = ck::registry::get(object);
        if (existing_entry != nullptr && existing_entry->status == Status::Deleted) {
            ck_object::unset_hidden(object);
            g_object_registry.erase(object);

            logger.debug("Restored object visibility, erased from registry ({})", existing_entry->mod_id);
        }
    }

    void clear_for_mod(const char *mod_id) {
        int restored_count = 0;

        for (auto it = g_object_registry.begin(); it != g_object_registry.end();) {
            const auto &entry = it->second;

            if (entry.is_deleted() && entry.mod_id == mod_id) {
                if (entry.object != nullptr) {
                    ck_object::unset_hidden(entry.object);
                    restored_count++;
                }

                it = g_object_registry.erase(it);
            } else {
                ++it;
            }
        }

        if (restored_count > 0) {
            logger.info("Hot Reload: Restored {} deleted/hidden objects for mod '{}'", restored_count, mod_id);
        }
    }

    void unhide() {
        int restored_count = 0;

        for (const auto &[object, entry] : g_object_registry) {
            if (entry.is_deleted() && entry.object != nullptr) {
                ck_object::unset_hidden(entry.object);
                logger.debug("restored hidden object for mod: {}", entry.mod_id);
                restored_count++;
            }
        }

        if (restored_count > 0) { logger.info("Restored {} hidden objects", restored_count); }
    }

    void hide() {
        int hidden_count = 0;

        for (const auto &[object, entry] : g_object_registry) {
            if (entry.is_deleted() && entry.object != nullptr) {
                ck_object::set_hidden(entry.object);
                hidden_count++;
            }
        }

        if (hidden_count > 0) { logger.info("Hidden {} objects", hidden_count); }
    }
}
