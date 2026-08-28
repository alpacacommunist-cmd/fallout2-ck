#include "ck_registry.h"
#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Registry [Deleted]");

namespace ck::common {
    const char* system_mod_id();
    const char* current_mod_id();
}

namespace ck::registry::deleted {
    void add(fallout::Object* obj) {
        if (obj == nullptr) return;

		const char* current_mod_id = ck::common::current_mod_id();
		std::string mod_id = current_mod_id != nullptr ? current_mod_id : ck::common::system_mod_id();

		obj->flags |= fallout::OBJECT_HIDDEN;

        g_deleted_objects.push_back(CkDeletedObject{ obj, mod_id });
    }

	void clear_for_mod(std::string_view mod_id) {
        int restored_count = 0;

        std::erase_if(g_deleted_objects, [mod_id, &restored_count](const CkDeletedObject& entry) {
            if (entry.mod_id == mod_id) {
                if (entry.ptr != nullptr) {
                    entry.ptr->flags &= ~fallout::OBJECT_HIDDEN;
                    restored_count++;
                }
                return true;
            }
            return false;
        });

        if (restored_count > 0) {
            log.info("Hot Reload: Restored {} deleted/hidden objects for mod '{}'", restored_count, mod_id);
        }
    }

    void unhide() {
		int restored_count = 0;
		for (const auto& entry : g_deleted_objects) {
			if (entry.ptr != nullptr) {
				entry.ptr->flags &= ~fallout::OBJECT_HIDDEN;
				log.debug("restored hidden object for mod: {}", entry.mod_id);
				restored_count++;
			}
		}

		if (restored_count > 0) {
			log.info("Restored {} hidden objects", restored_count);
		}
    }

    void hide() {
		int hidden_count = 0;
        for (const auto& entry : g_deleted_objects) {
            if (entry.ptr != nullptr) {
                entry.ptr->flags |= fallout::OBJECT_HIDDEN;
				hidden_count++;
            }
        }

		if (hidden_count > 0) {
			log.info("Hidden {} objects", hidden_count);
		}
    }
}
