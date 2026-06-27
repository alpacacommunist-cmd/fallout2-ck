#ifndef CK_OBJECT_REGISTRY_H
#define CK_OBJECT_REGISTRY_H

#include "ck_api.h"

#include <unordered_map>
#include <string>

namespace fallout {
    struct Object;
	int reg_anim_clear(Object* a1);
}

struct LuaMeta {
	int         proto_sid;
	std::string tag;
	std::string mod_id;
};

struct CkManagedObject {
    fallout::Object* ptr    = nullptr;
    int              lua_id = -1;
    bool             alive  = true;

	LuaMeta   meta;
};

class CkObjectRegistry {
public:
    int  add(fallout::Object* obj, const LuaMeta& meta = {});
	void destroy_objects_for_mod(const char* target_mod_id);
	int remove_by_ptr(fallout::Object* ptr);
	void ck_set_mod_context(const char* mod_id);
	const char* get_current_mod_id();

	const CkManagedObject* get_managed(int lua_id) const;

    fallout::Object* get(int lua_id) const;
    int find_by_ptr(fallout::Object* ptr) const;
	const LuaMeta* get_meta(int lua_id) const;

    void destroy_all();
    void clear();

	std::string current_mod_id;

private:
    int next_id = 1;
    std::unordered_map<int, CkManagedObject> objects;
};

extern CkObjectRegistry gObjectRegistry;

CK_API void ck_set_mod_context(const char* mod_id);
CK_API const char* ck_get_current_mod_id();

#endif
