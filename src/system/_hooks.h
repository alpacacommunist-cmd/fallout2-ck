#ifndef CK_HOOKS_PRIVATE_H
#define CK_HOOKS_PRIVATE_H

namespace fallout {
    struct Object;
    struct Script;

	bool _combat_reload_map();
    void mapEdgeFree();
}

namespace ck::config {
    bool apply_worldmap_patches();
}

namespace ck::mods {
    const char* system_mod_id();
    const char* current_mod_id();
}

namespace ck::state {
    bool load(const char* path);
    void save(const char* path);
}

namespace ck::common {
    bool game_is_loading();
    bool in_combat();
}

namespace ck::proto::registry {
    enum class SyncMode {
        Prepare,
        Restore
    };

    void clear();
    void clear_for_object_type(int object_type);
    int get_pid_by_sid(int sid);

    namespace item {
        void sync_custom_items_on_map(SyncMode mode);
    }
}

#endif
