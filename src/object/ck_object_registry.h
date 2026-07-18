#ifndef CK_OBJECT_REGISTRY_H
#define CK_OBJECT_REGISTRY_H

#include <unordered_map>
#include <vector>
#include <string>

namespace fallout {
    struct Object;
	struct Rect;

	Object* _obj_blocking_at(Object* excludeObj, int tile_num, int elev);
	int objectDestroy(Object* object, Rect* rect);
    int reg_anim_clear(Object* obj);
}

struct LuaMeta {
    int         proto_sid = -1;
    std::string tag;
    std::string mod_id;
    int         source_pid = -1;
};

struct CkManagedObject {
    fallout::Object* ptr    = nullptr;
    int              lua_id = -1;
    LuaMeta          meta;
};

struct CkHiddenObjectEntry {
    fallout::Object* ptr = nullptr;
    std::string mod_id;
};

namespace ck::registry {
    extern std::unordered_map<int, CkManagedObject> g_objects;
    extern std::vector<CkHiddenObjectEntry> g_hidden_objects;

    int  add(fallout::Object* obj, const LuaMeta& meta = {});
    int  remove_by_ptr(fallout::Object* ptr);
    void clear_resources_for_mod(const char* target_mod_id);
    void register_hidden_object(fallout::Object* obj, const std::string& mod_id);

    const CkManagedObject* get_managed(int lua_id);
    fallout::Object* get(int lua_id);
    int  find_by_ptr(fallout::Object* ptr);
    const LuaMeta* get_meta(int lua_id);
    void clear();

    void temporary_unhide_for_save();
    void rehide_after_save();
}

extern "C" void ck_registry_clear();

#endif
