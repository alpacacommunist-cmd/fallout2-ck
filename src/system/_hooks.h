#ifndef CK_HOOKS_PRIVATE_H
#define CK_HOOKS_PRIVATE_H

namespace fallout {
    struct Object;
    struct Script;

	bool _combat_reload_map();
    void mapEdgeFree();
}

namespace ck::script {
	fallout::Script* get_dummy(int sid);
	void reset();
    void reset_vars(fallout::Script* script);
}

namespace ck::config {
    bool apply_worldmap_patches();
}

namespace ck::state {
    bool load(const char* path);
    void save(const char* path);
}

namespace ck::common {
    bool game_is_loading();
    bool in_combat();
}

namespace ck::registry {
	void clear_context();
    void clear();

    namespace deleted {
        void unhide();
        void hide();
    }

    namespace modified {
		void restore_sids();
		void reapply_sids();
    }

    namespace created {
        int  remove_by_ptr(fallout::Object* ptr);
    }
}


namespace ck::proto::registry {
    enum class SyncMode {
        Prepare,
        Restore
    };

    namespace item {
        void sync_custom_items_on_map(SyncMode mode);
    }

    void clear();
}

#endif
