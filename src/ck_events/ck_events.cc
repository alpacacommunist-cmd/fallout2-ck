#include "ck_scripting.h"
#include "map/ck_map.h"

#include "ck_proto/cache/ck_proto_cache.h"
#include "ck_proto/registry/ck_proto_registry.h"

#include "ck_state/ck_state.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "ce_config/ck_config_maps.h"
#include "ce_config/ck_config_city.h"
#include "ce_config/ck_config_patch.h"

#include "ck_i18n/ck_i18n.h"

#include "ck_registry/ck_registry.h"

#include "ck_lua_proxy/ck_lua_proxy.h"

#include "settings.h"

#include "log.h"
static logging::Context logger{"ck_events.cc", logging::Source::Native};

// LUA calls
namespace ck::proxy::detail {
    extern int bootstrap;
    extern int set_language;
    extern int clear_registries;
    extern int map_exit;
}

// Helper functions
namespace ck::events {
    static bool is_test_mode = false;
    static std::string g_test_suite_name = std::string();

    std::string& current_test_suite_name() {
        return g_test_suite_name;
    }

    void set_test_suite(const char* name) {
        g_test_suite_name = std::string(name);
    }

    static void ck_set_language() {
        logger.warning.banner("System language: {}", fallout::settings.system.language);
        ck::i18n::load_language(fallout::settings.system.language);
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::set_language, fallout::settings.system.language);
    }
}

// System events
namespace ck::events::system {
    // Init
    static int    g_game_argc = 0;
    static char** g_game_argv = nullptr;
    void init(int argc, char** argv) {
        logger.info("Initializing LuaJIT backend...");

        g_game_argc = argc; g_game_argv = argv;

        for (int index = 1; index < g_game_argc; index++) {
            std::string arg = g_game_argv[index];
            if (arg == "--test" || arg == "--integration-tests") {
                is_test_mode = true;

                if (index + 1 < g_game_argc) g_test_suite_name = g_game_argv[index + 1];
                break;
            }
        }

        if (is_test_mode) {
            logger.info("LAUNCHING IN INTEGRATION TEST MODE: {}", g_test_suite_name);
        }

        // Parse maps.txt, city.txt to get total count and cache names for validations
        ck::config::maps::preprocess_maps();
        ck::config::city::preprocess_areas();
        logger.info("Maps count in maps.txt: {}", ck::config::maps::next_index() - 1);
        logger.info("Areas count in city.txt: {}", ck::config::city::next_index() - 1);

        ck::proxy::init_lua_state("../?.lua;../?/init.lua");
        ck::proxy::cache_functions();
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::bootstrap);
    }

    void game_start() {
        ck::dispatcher::on_game_start();
    }

    void engine_ready() {
        logger.debug("engine_ready");
        ck_set_language();

        if (is_test_mode) {
            fallout::settings.ui.skip_opening_movies = 1;
        }

        if (!is_test_mode) {
            gProtoCache.initialize("build/proto_cache.db");
        }

        ck::config::apply_worldmap_patches();

        ck::dispatcher::on_engine_ready();
    }

    // Scripts reset, e.g exit to splash screen
    void scripts_reset() {
        logger.info("ck_on_scripts_reset");

        ck::registry::clear();
        ck::common::clear_lua_registries();
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
namespace ck::events::gameplay {
}

