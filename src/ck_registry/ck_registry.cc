#include "ck_registry.h"
#include <algorithm>

#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Registry");

namespace ck::registry {
    std::unordered_map<int, CkCreatedObject> g_created_objects;
    std::vector<CkDeletedObject>             g_deleted_objects;

    std::unordered_map<fallout::Object*, int> g_ptr_to_lua_id;

	static int g_next_id = 1;

	int  next_lua_id() { return g_next_id++; }
	void reset_lua_id_counter() { g_next_id = 1; }

    void clear_resources_for_mod(const char* target_mod_id) {
        if (target_mod_id == nullptr) return;
        std::string mod_str(target_mod_id);

        int restored_count = 0;
        auto hidden_it = g_deleted_objects.begin();
        while (hidden_it != g_deleted_objects.end()) {
            if (hidden_it->mod_id == mod_str) {
                if (hidden_it->ptr != nullptr) {
                    hidden_it->ptr->flags &= ~fallout::OBJECT_HIDDEN;
                    restored_count++;
                }
                hidden_it = g_deleted_objects.erase(hidden_it);
            } else {
                ++hidden_it;
            }
        }

        if (restored_count > 0) {
            log.info("Hot Reload: Restored {} hidden map objects for mod '{}'", restored_count, mod_str);
        }

        std::vector<fallout::Object*> to_destroy;
        for (const auto& [id, managed] : g_created_objects) {
            if (managed.meta.mod_id == mod_str && managed.ptr != nullptr) {
                to_destroy.push_back(managed.ptr);
            }
        }

        for (fallout::Object* obj : to_destroy) {
            fallout::reg_anim_clear(obj);
            fallout::objectDestroy(obj, nullptr);
        }

        std::erase_if(g_created_objects, [&mod_str](const auto& item) {
            return item.second.meta.mod_id == mod_str;
        });

        if (!to_destroy.empty()) {
            log.info("Hot Reload: Physically destroyed {} temporary objects for mod '{}'", to_destroy.size(), mod_str);
        }
    }

	void on_map_exit() {
		deleted::unhide();

		clear();
	}

	void clear() {
        g_created_objects.clear();
        g_deleted_objects.clear();
        g_ptr_to_lua_id.clear();

		reset_lua_id_counter();

        log.info("Cleared object registry entirely.");
	}
}

void ck_registry_clear() {
    ck::registry::clear();
}
