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

struct CkModifiedObject {
    fallout::Object* ptr    = nullptr;
    int              lua_id = -1;
    LuaMeta          meta;
};

struct CkDeletedObject {
    fallout::Object* ptr = nullptr;
    std::string mod_id;
};

namespace ck::registry {
    extern std::unordered_map<int, CkCreatedObject>  g_created_objects;
	extern std::unordered_map<int, CkModifiedObject> g_modified_objects;
    extern std::vector<CkDeletedObject>              g_deleted_objects;

    extern std::unordered_map<fallout::Object*, int> g_ptr_to_lua_id;

	int next_lua_id();
	void reset_lua_id_counter();

	int find_by_ptr(fallout::Object* ptr);
	const LuaMeta* get_meta(int lua_id);

	fallout::Object* get_object(int lua_id);

    void clear();
    void clear_resources_for_mod(const char* target_mod_id);

	void on_map_exit();

    namespace created {
        int  add(fallout::Object* obj, LuaMeta meta = {});
        int  remove_by_ptr(fallout::Object* ptr);
		void clear_for_mod(std::string_view mod_id);

        fallout::Object* get_object(int lua_id);
        const CkCreatedObject* get(int lua_id);
    }

    namespace deleted {
        void add(fallout::Object* obj);
		void clear_for_mod(std::string_view mod_id);

        void unhide();
        void hide();
    }

	namespace modified {
		int  add(fallout::Object* obj, LuaMeta meta = {});
		int  restore(fallout::Object* object);
		void clear_for_mod(std::string_view mod_id);

		void restore_sids();
		void reapply_sids();

		const CkModifiedObject* get(int lua_id);
	}
}

CK_API void ck_registry_clear();
CK_API void ck_registry_clear_for_mod(const char* target_mod_id);

CK_API int  ck_registry_modify_object(void* ptr);
CK_API bool ck_registry_object_is_modified(int lua_id);
CK_API bool ck_registry_object_is_created(int lua_id);
CK_API int  ck_registry_restore_modified_object(void* ptr);

CK_API bool ck_registry_bind_to_object(int lua_id);

CK_API const char* ck_registry_object_get_mod_id(fallout::Object* object);
CK_API const char* ck_registry_object_get_lua_tag(fallout::Object* object);

#endif
