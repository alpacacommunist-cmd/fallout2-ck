#include "system/_init.h"

#include "ce_config/ck_config_maps.h"
#include "ck_i18n/ck_i18n.h"
#include "ce_config/ck_config_city.h"

#include "ck_lua_proxy/ck_lua_proxy.h"

#include "settings.h"

#include "log.h"
static logging::Context logger{"ck_init.cc", logging::Source::Native};

namespace ck::proxy::detail {
    extern int bootstrap;
    extern int set_language;
}

namespace ck::init {
    static bool is_test_mode = false;
    static std::string test_suite_name = std::string();

    bool test_mode_enabled() {
        return is_test_mode;
    }

    std::string& current_test_suite_name() {
        return test_suite_name;
    }

    void set_test_suite(const char* name) {
        test_suite_name = std::string(name);
    }

    void set_language() {
        logger.warning.banner("System language: {}", fallout::settings.system.language);
        ck::i18n::load_language(fallout::settings.system.language);
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::set_language, fallout::settings.system.language);
    }
}

namespace ck::hooks::system {
    // Init
    static int    g_game_argc = 0;
    static char** g_game_argv = nullptr;

    void init(int argc, char** argv) {
        logger.info("Initializing LuaJIT backend...");

        // FFI calls aren't used directly in cpp code so compiler needs to know it's not a dead code
        ck::ffi::init_ffi_manifest();

        g_game_argc = argc; g_game_argv = argv;

        for (int index = 1; index < g_game_argc; index++) {
            std::string arg = g_game_argv[index];
            if (arg == "--test" || arg == "--integration-tests") {
                ck::init::is_test_mode = true;

                if (index + 1 < g_game_argc) ck::init::test_suite_name = g_game_argv[index + 1];
                break;
            }
        }

        if (ck::init::is_test_mode) {
            logger.info("LAUNCHING IN INTEGRATION TEST MODE: {}", ck::init::test_suite_name);
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
}

