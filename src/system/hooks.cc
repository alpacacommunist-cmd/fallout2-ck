#include "system/init.h"

#include "map/ck_map.h"

#include "ck_proto/cache/ck_proto_cache.h"
#include "ck_proto/registry/ck_proto_registry.h"

#include "ck_state/ck_state.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "ce_config/ck_config_patch.h"

#include "ck_registry/ck_registry.h"

#include "ck_lua_proxy/ck_lua_proxy.h"

#include "settings.h"

#include "log.h"
static logging::Context logger{"ck_events.cc", logging::Source::Native};

// LUA calls
namespace ck::proxy::detail {
    extern int clear_registries;
    extern int map_exit;
}

// System events
namespace ck::hooks::system {
    void game_start() {
        ck::dispatcher::on_game_start();
    }

    void engine_ready() {
        logger.debug("engine_ready");

        ck::init::set_language();

        if (ck::init::test_mode_enabled()) {
            // TEST mode
            fallout::settings.ui.skip_opening_movies = 1;
        } else {
            // NORMAL start
            gProtoCache.initialize("build/proto_cache.db");
        }

        ck::config::apply_worldmap_patches();

        ck::dispatcher::on_engine_ready();
    }

    // Scripts reset, e.g exit to splash screen
    void scripts_reset() {
        logger.info("ck_on_scripts_reset");

        ck::registry::clear();
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::clear_registries);
    }

    // loadsave.cc
    void before_game_save() {
        logger.debug("on_before_game_save");

        ck::registry::deleted::unhide();
        ck::registry::modified::restore_sids();
        ck::proto::item::sync_custom_items_on_map(ck::proto::SyncMode::Prepare);
    }

    void game_saved(const char* path) {
        logger.debug("on_game_save");

        ck_state_save(path);

        ck::registry::deleted::hide();
        ck::registry::modified::reapply_sids();
        ck::proto::item::sync_custom_items_on_map(ck::proto::SyncMode::Restore);
    }

    void before_game_load(const char* path) {
        logger.debug("ck_scripting_on_before_game_load");

        // aka on_map_exit
        ck::map::on_before_map_load();
        ck_state_load(path);
    }

    void game_loaded() {
        ck::dispatcher::on_game_loaded();
        ck::map::on_map_enter();
    }

    void worldmap_open() {
        logger.debug("worldmap_open");
        ck::map::on_before_map_load();
    }

    // Separate hook for destroying objects to help clearing out ptrs
    void object_destroyed(fallout::Object* object) {
        logger.debug("object_destroyed");
        ck::registry::created::remove_by_ptr(object);
    }

    // Exit
    void exit() {
        ck::proxy::shutdown();

        logger.info("ck_scripting_exit");
        logger.info("Shutting down LuaJIT backend...");
    }
}

// Gameplay events
namespace ck::hooks::gameplay {
}

