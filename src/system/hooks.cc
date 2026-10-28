#include "ck_ids.h"
#include "system/_hooks.h"
#include "system/init.h"
#include "system/rendering.h"
#include "debug/overlay.h"
#include "utils/fallout/types/object.h"

#include "ck_proto/cache/ck_proto_cache.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_dispatcher/ck_dispatcher.h"
#include "ck_registry/ck_registry.h"
#include "script/ck_script.h"

#include "fallout2-ce/settings.h"

#include "log.h"
static logging::Context logger{"system/hooks/system.cc", logging::Source::Native};

namespace ck_object = ck::utils::fallout::types::object;

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
        // Resets critter proto, script indexes
        ck::proto::registry::clear_for_object_type(static_cast<int>(ck::ids::ObjectType::CRITTER));
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
        ck::proto::registry::item::sync_custom_items_on_map(ck::proto::registry::SyncMode::Prepare);
    }

    void game_saved(const char* path) {
        logger.debug("game_saved");

        ck::state::save(path);

        ck::registry::deleted::hide();
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

    // alternative map updater
    // scripts.cc: static void _doBkProcesses()
	void map_update(unsigned int ticks) {
        ck::dispatcher::on_map_update(ticks);
	}

    // set localVarsCount to 0 bc script is fake (isn't physically loaded in gScriptsListEntries)
    // scripts.cc: int _scr_find_str_run_info
    void proto_script_requested(fallout::Script* script) {
        ck::script::reset_vars(script);
    }

    bool handle_proc(int sid, fallout::Script* script, int proc) {
        // skip if script isn't processed
        if (!ck::ids::is_ck_script_index(ck::script::index(script))) return false;
        // skip map updates (if proc disabled)
        if (proc == fallout::SCRIPT_PROC_CRITTER && !proc_enabled(script, fallout::SCRIPT_PROC_CRITTER)) return false;

        bool handled = false;

        int lua_id = ck::registry::find_by_ptr(ck::script::owner(script));
        if (lua_id != -1) {
            const LuaMeta* meta = ck::registry::get_meta(lua_id);
            handled = ck::dispatcher::on_proc(lua_id, proc, ck::script::param(script), meta->mod_id);
        } else {
            int pid = ck::proto::registry::get_pid_by_sid(ck::script::index(script));
            if (pid == -1) return false;

            handled = ck::dispatcher::on_proto_proc(pid, proc, ck::script::param(script), ck::mods::system_mod_id());
        }

        if (handled) {
            ck::script::overrides(script, 1);
            return true;
        } else {
            logger.warning("unhandled proc: {} (script_index: {})", proc, ck::script::index(script));
        }

        return false;
    };


    // sets last dialog choice (used for controlling dialogs)
    // game_dialog.cc: int _gdProcessChoice(int optionIndex)
    void last_dialog_choice(int option_index) {
        ck::script::set_last_dialog_choice(option_index);
    }

    // Separate hook for destroying objects to help clearing out ptrs
    // object.cc: int objectDestroy(Object* object, Rect* rect)
    void object_destroyed(fallout::Object* object) {
        logger.debug("object_destroyed");
        ck::registry::created::remove_by_ptr(object);
    }

    // Checks whether items are identical
    // items.cc: static bool _item_identical
    bool items_identical_check(fallout::Object* item1, fallout::Object* item2) {
        return ck_object::pid(item1) == ck_object::pid(item2);
    }
}
