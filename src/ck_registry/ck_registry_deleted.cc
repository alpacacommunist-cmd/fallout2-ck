#include "ck_registry.h"
#include "obj_types.h"

namespace ck::registry::deleted {
    void add(fallout::Object* obj, const std::string& mod_id) {
        if (obj == nullptr) return;
        g_deleted_objects.push_back(CkDeletedObject{ obj, mod_id });
    }

    void unhide_for_save() {
        for (const auto& entry : g_deleted_objects) {
            if (entry.ptr != nullptr) {
                entry.ptr->flags &= ~fallout::OBJECT_HIDDEN;
            }
        }
    }

    void rehide_after_save() {
        for (const auto& entry : g_deleted_objects) {
            if (entry.ptr != nullptr) {
                entry.ptr->flags |= fallout::OBJECT_HIDDEN;
            }
        }
    }
}
