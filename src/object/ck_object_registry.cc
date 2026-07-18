#include "object/ck_object.h"
#include "object/ck_object_registry.h"
#include <algorithm>

#include "obj_types.h"

#include "ck_log.h"
static const Logger log("CK Object Registry");

namespace {
    int next_id = 1;
}

namespace ck::registry {
    std::unordered_map<int, CkManagedObject> g_objects;
    std::vector<CkHiddenObjectEntry> g_hidden_objects;

    int add(fallout::Object* obj, const LuaMeta& meta) {
        int id = next_id++;
        g_objects[id] = CkManagedObject{ obj, id, meta };
        return id;
    }

    void register_hidden_object(fallout::Object* obj, const std::string& mod_id) {
        if (obj == nullptr) return;
        g_hidden_objects.push_back(CkHiddenObjectEntry{ obj, mod_id });
    }

    void clear_resources_for_mod(const char* target_mod_id) {
        if (target_mod_id == nullptr) return;
        std::string mod_str(target_mod_id);

        int restored_count = 0;
        auto hidden_it = g_hidden_objects.begin();
        while (hidden_it != g_hidden_objects.end()) {
            if (hidden_it->mod_id == mod_str) {
                if (hidden_it->ptr != nullptr) {
                    hidden_it->ptr->flags &= ~fallout::OBJECT_HIDDEN;
                    restored_count++;
                }
                hidden_it = g_hidden_objects.erase(hidden_it);
            } else {
                ++hidden_it;
            }
        }

        if (restored_count > 0) {
            log.info("Hot Reload: Restored {} hidden map objects for mod '{}'", restored_count, mod_str);
        }

        std::vector<fallout::Object*> to_destroy;
        for (const auto& [id, managed] : g_objects) {
            if (managed.meta.mod_id == mod_str && managed.ptr != nullptr) {
                to_destroy.push_back(managed.ptr);
            }
        }

        for (fallout::Object* obj : to_destroy) {
            fallout::reg_anim_clear(obj);
            fallout::objectDestroy(obj, nullptr);
        }

        if (!to_destroy.empty()) {
            log.info("Hot Reload: Physically destroyed {} temporary objects for mod '{}'", to_destroy.size(), mod_str);
        }
    }

    int remove_by_ptr(fallout::Object* ptr) {
        if (ptr == nullptr) return -1;

        int deleted_id = -1;
        std::string obj_tag;

        std::erase_if(g_objects, [ptr, &deleted_id, &obj_tag](const auto& item) {
            if (item.second.ptr == ptr) {
                deleted_id = item.second.lua_id;
                obj_tag = item.second.meta.tag;

                return true;
            }

            return false;
        });

        if (deleted_id != -1) {
            log.debug("Engine destroyed object [ID: {}, Tag: '{}']. Registry size: {}",
                      deleted_id, obj_tag.empty() ? "untagged" : obj_tag, g_objects.size());
        }

        return deleted_id;
    }

    const CkManagedObject* get_managed(int lua_id) {
        auto it = g_objects.find(lua_id);
        return (it == g_objects.end()) ? nullptr : &it->second;
    }

    fallout::Object* get(int lua_id) {
        auto it = g_objects.find(lua_id);
        return (it == g_objects.end()) ? nullptr : it->second.ptr;
    }

    int find_by_ptr(fallout::Object* ptr) {
        for (const auto& [id, managed] : g_objects) {
            if (managed.ptr == ptr) return id;
        }
        return -1;
    }

    const LuaMeta* get_meta(int lua_id) {
        auto it = g_objects.find(lua_id);
        return (it == g_objects.end()) ? nullptr : &it->second.meta;
    }

    void clear() {
		int restored_count = 0;
		for (const auto& entry : g_hidden_objects) {
			if (entry.ptr != nullptr) {
				entry.ptr->flags &= ~fallout::OBJECT_HIDDEN;
				restored_count++;
			}
		}

		if (restored_count > 0) {
			log.info("Registry Clear Guard: Restored {} hidden objects before map unload/save", restored_count);
		}

        g_objects.clear();
        g_hidden_objects.clear();
        next_id = 1;
        log.info("Cleared object registry entirely.");
    }

    void temporary_unhide_for_save() {
        for (const auto& entry : g_hidden_objects) {
            if (entry.ptr != nullptr) entry.ptr->flags &= ~fallout::OBJECT_HIDDEN;
        }
    }

    void rehide_after_save() {
        for (const auto& entry : g_hidden_objects) {
            if (entry.ptr != nullptr) entry.ptr->flags |= fallout::OBJECT_HIDDEN;
        }
    }
}

void ck_registry_clear() {
    ck::registry::clear();
}
