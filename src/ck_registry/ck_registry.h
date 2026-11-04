#ifndef CK_REGISTRY_H
#define CK_REGISTRY_H

#include <unordered_map>
#include "ck_api.h"

namespace fallout {
    struct Object;
    struct Rect;
}

namespace ck::registry {
    enum class Status {
        Created,
        Modified,
        Deleted
    };

    struct RegistryEntry {
        fallout::Object* object = nullptr;
        const char* mod_id = nullptr;

        // "created" only
        std::string tag;

        Status status;

        inline bool is_created() const  { return status == Status::Created; }
        inline bool is_modified() const { return status == Status::Modified; }
        inline bool is_deleted() const  { return status == Status::Deleted; }
    };

    extern std::unordered_map<fallout::Object*, RegistryEntry> g_object_registry;

	const RegistryEntry* get(fallout::Object* object);

	int find_by_ptr(fallout::Object* ptr);

    void clear();
    void clear_resources_for_mod(const char* target_mod_id);

	void clear_context();

    namespace created {
        int add(fallout::Object *object, const char *tag, const char *mod_id);
        int remove_by_ptr(fallout::Object* object);
		void clear_for_mod(const char* mod_id);
    }

    namespace deleted {
        void add(fallout::Object* object);
        void remove(fallout::Object* object);

		void clear_for_mod(const char* mod_id);

        void unhide();
        void hide();
    }

	namespace modified {
        int add(fallout::Object *object, const char *mod_id);
		bool restore(fallout::Object* object);
		void clear_for_mod(const char* mod_id);
        void free_script_indexes();
	}
}

CK_API void ck_registry_clear();

CK_API int ck_registry_modify_object(fallout::Object* object);
CK_API bool ck_registry_object_is_modified(fallout::Object* object);
CK_API bool ck_registry_object_is_created(fallout::Object* object);
CK_API bool ck_registry_restore_modified_object(fallout::Object* object);

CK_API bool ck_registry_bind_to_object(fallout::Object* object);

CK_API const char* ck_registry_object_get_mod_id(fallout::Object* object);
CK_API const char* ck_registry_object_get_lua_tag(fallout::Object* object);

#endif
