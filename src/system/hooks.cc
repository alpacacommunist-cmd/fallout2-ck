#include "system/_hooks.h"
#include "system/init.h"
#include "system/rendering.h"
#include "debug/overlay.h"

#include "ck_proto/cache/ck_proto_cache.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "fallout2-ce/settings.h"

#include "log.h"
static logging::Context logger{"system/hooks/system.cc", logging::Source::Native};

// LUA calls
namespace ck::proxy::detail {
    extern int clear_registries;
    extern int map_exit;
}

namespace ck::hooks {
    static void clear_runtime_context() {
        // Custom proto items PID is temporary, restore it to SOURCE_PID before leaving the map
        // so that map save file holds vanilla data
        if (!ck::common::game_is_loading()) {
            ck::proto::registry::item::sync_custom_items_on_map(proto::registry::SyncMode::Prepare);
        }

        // Resets lua registries, updates state when leaving a map
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::map_exit);
        // Resets custom object scripts
        ck::script::reset();
        // Restores original objects SID/PID/flags for [modified]/[deleted] objects, clears [created] pointers
        ck::registry::clear_context();

        // Resets rendering
        fallout::mapEdgeFree();
        ck::rendering::clear();

        if (ck::debug::enabled()) ck::debug::toggle();
    }

    static void init_runtime_context() {
        // Restore proto items PID (stored in `id`)
        ck::proto::registry::item::sync_custom_items_on_map(ck::proto::registry::SyncMode::Restore);

        // Dispatch event
        // some events like map_enter have additional logic defined in events.lua
        ck::dispatcher::on_map_enter();

        ck::rendering::refresh();
        if (ck::common::in_combat()) fallout::_combat_reload_map();
    }
}

namespace ck::hooks::system {
    void game_start() {
        ck::dispatcher::on_game_start();
    }

    void engine_ready() {
        logger.debug("engine_ready");

        ck::init::set_language();
        if (ck::init::test_mode_enabled()) {
            fallout::settings.ui.skip_opening_movies = 1;
        } else {
            gProtoCache.initialize("build/proto_cache.db");
        }

        ck::config::apply_worldmap_patches();
        ck::dispatcher::on_engine_ready();
    }

    // Scripts reset, e.g exit to splash screen
    void scripts_reset() {
        logger.debug("scripts_reset");

        ck::registry::clear();
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::clear_registries);
    }

    // loadsave.cc
    void before_game_save() {
        logger.debug("before_game_save");

        ck::registry::deleted::unhide();
        ck::registry::modified::restore_sids();
        ck::proto::registry::item::sync_custom_items_on_map(ck::proto::registry::SyncMode::Prepare);
    }

    void game_saved(const char* path) {
        logger.debug("game_saved");

        ck::state::save(path);

        ck::registry::deleted::hide();
        ck::registry::modified::reapply_sids();
        ck::proto::registry::item::sync_custom_items_on_map(ck::proto::registry::SyncMode::Restore);
    }

    void before_game_load(const char* path) {
        logger.debug("before_game_load");

        ck::hooks::clear_runtime_context();
        ck::state::load(path);
    }

    void game_loaded() {
        logger.debug("game_loaded");

        ck::dispatcher::on_game_loaded();
        ck::hooks::init_runtime_context();
    }

    // Exit
    void exit() {
        logger.info("exit");

        ck::proxy::shutdown();
        ck::proto::registry::clear();
    }
}

namespace ck::hooks::gameplay {
    // worldmap.cc
    void worldmap_open() {
        logger.debug("worldmap_open");

        ck::hooks::clear_runtime_context();
    }

    // map.cc
    void before_map_load() {
        logger.debug("before_map_load");

        ck::hooks::clear_runtime_context();
    }

    void map_enter() {
        logger.debug("map_enter");

        ck::hooks::init_runtime_context();
    }

    // script.cc
	void time_advance(int hours, int minutes) {
        logger.debug("time_advance");

        ck::dispatcher::on_time_advance(hours, minutes);
	}
    void after_rest(int hours, int minutes) { time_advance(hours, minutes); }

	void day_passed() {
        logger.debug("day_passed");

        ck::dispatcher::on_day_passed();
	}

	void map_update(unsigned int ticks) {
        ck::dispatcher::on_map_update(ticks);
	}

    // Object requests script
    // scripts.cc: int scriptGetScript
    void script_requested(fallout::Script** script_ptr, int sid) {
        *script_ptr = ck::script::get_dummy(sid);
    }

    // set localVarsCount to 0 bc script is fake
    // scripts.cc: int _scr_find_str_run_info
    void proto_script_requested(fallout::Script* script) {
        ck::script::reset_vars(script);
    }

    // Separate hook for destroying objects to help clearing out ptrs
    // object.cc: int objectDestroy(Object* object, Rect* rect)
    void object_destroyed(fallout::Object* object) {
        logger.debug("object_destroyed");
        ck::registry::created::remove_by_ptr(object);
    }
}
