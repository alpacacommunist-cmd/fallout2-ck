#ifndef CK_REGISTRY_H
#define CK_REGISTRY_H

#include <unordered_map>
#include <vector>
#include <string>
#include "ck_api.h"

namespace fallout {
    struct Object;
    struct Rect;
    int objectDestroy(Object* object, Rect* rect);
    int reg_anim_clear(Object* obj);
}

struct LuaMeta {
    std::string mod_id;
    std::string tag;

    int         source_pid = -1;
    int         source_sid = -1;
};

struct CkCreatedObject {
    fallout::Object* ptr    = nullptr;
    int              lua_id = -1;
    LuaMeta          meta;
};

struct CkDeletedObject {
    fallout::Object* ptr = nullptr;
    std::string mod_id;
};

namespace ck::registry {
    extern std::unordered_map<int, CkCreatedObject> g_created_objects;
    extern std::vector<CkDeletedObject>             g_deleted_objects;
    extern std::unordered_map<fallout::Object*, int> g_ptr_to_lua_id;

    void clear();
    void clear_resources_for_mod(const char* target_mod_id);

    namespace created {
        int  add(fallout::Object* obj, const LuaMeta& meta = {});
        int  remove_by_ptr(fallout::Object* ptr);

        fallout::Object* get_object(int lua_id);
        const CkCreatedObject* get(int lua_id);
        int  find_by_ptr(fallout::Object* ptr);
        const LuaMeta* get_meta(int lua_id);
    }

    namespace deleted {
        void add(fallout::Object* obj, const std::string& mod_id);
        void unhide_for_save();
        void rehide_after_save();
    }
}

CK_API void ck_registry_clear();

#endif
