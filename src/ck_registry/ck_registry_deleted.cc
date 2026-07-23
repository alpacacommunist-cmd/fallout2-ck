#include "ck_registry.h"
#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Registry [Deleted]");

namespace ck::registry::deleted {
    void add(fallout::Object* obj) {
        if (obj == nullptr) return;

		const char* mod_id = ck_get_current_mod_id();
		obj->flags |= fallout::OBJECT_HIDDEN;

        g_deleted_objects.push_back(CkDeletedObject{ obj, mod_id });
    }

    void unhide() {
		int restored_count = 0;
		for (const auto& entry : g_deleted_objects) {
			if (entry.ptr != nullptr) {
				entry.ptr->flags &= ~fallout::OBJECT_HIDDEN;
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
