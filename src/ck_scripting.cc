#include <cstring>

#include "ck_scripting.h"
#include "ck_encoding.h"

#include "ck_registry/ck_registry.h"
#include "ck_proto/cache/ck_proto_cache.h"
#include "ck_proto/registry/ck_proto_registry.h"

#include "ck_state/ck_state.h"
#include "ck_dispatcher/ck_dispatcher.h"
#include "ck_i18n/ck_i18n.h"

#include "ck_lua_proxy/ck_lua_proxy.h"

#include "settings.h"
#include "obj_types.h"

#include "ck_log.h"
static const Logger logger("CK Scripting");

namespace fallout {
    enum Color : unsigned char;
    enum CombatState : unsigned int {
        COMBAT_STATE_OUT_COMBAT = 0x00, COMBAT_STATE_IN_COMBAT = 0x01,
        COMBAT_STATE_PLAYER_TURN = 0x02, COMBAT_STATE_EXIT_REQUESTED = 0x08,
    };

    extern int gElevation;

	void tileWindowRefreshRect(Rect* rect, int elevation);
    int textObjectAdd(Object* object, char* string, int font, Color color, Color outlineColor, Rect* rect);

	void displayMonitorAddMessage(const char* str);
    int soundPlayFile(const char* name);

    extern CombatState gCombatState;
}

namespace ck {
	void on_map_enter();

    namespace proxy::detail {
        extern int reload_mods;
        extern int bootstrap;
        extern int set_language;
    }
}

void ck_print_monitor_message(const char* message) {
    std::string converted = utf8_to_cp1251(std::string_view(message));
    fallout::displayMonitorAddMessage(converted.c_str());
}


static bool g_reloading_mods = false;
bool ck_reloading_mods() {
    return g_reloading_mods;
}

void ck_reload_mods() {
    if (ck_in_combat()) {
        fallout::displayMonitorAddMessage("Disabled in combat");
        return;
    }

	logger.info("ck_reload_mods");

    g_reloading_mods = true;
    ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::reload_mods);
    g_reloading_mods = false;
}

// Init

static bool is_test_mode = false;
static std::string g_test_suite_name = std::string();

static int    g_game_argc = 0;
static char** g_game_argv = nullptr;
void ck_scripting_init(int argc, char** argv) {
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

    ck::proxy::init_lua_state("../?.lua;../?/init.lua");
    ck::proxy::cache_functions();
    ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::bootstrap);
}

void ck_on_scripts_reset() {
	logger.info("ck_on_scripts_reset");
}

// Exit
void ck_scripting_exit() {
    ck::proxy::shutdown();

    logger.info("ck_scripting_exit");
    logger.info("Shutting down LuaJIT backend...");
}

void ck_scripting_set_language() {
    logger.info("System language: {}", fallout::settings.system.language);
    ck::i18n::load_language(fallout::settings.system.language);
    ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::set_language, fallout::settings.system.language);
}

void ck_scripting_on_game_start() {
    ck::dispatcher::on_game_start();
}

void ck_scripting_on_engine_ready() {
    logger.debug("ck_scripting_on_engine_ready");
	ck_scripting_set_language();

    if (is_test_mode) {
		fallout::settings.ui.skip_opening_movies = 1;
	}

    if (!is_test_mode) {
        gProtoCache.initialize("build/proto_cache.db");
    }

    ck::dispatcher::on_engine_ready();
}

void ck_scripting_on_object_destroyed(fallout::Object* object) {
    logger.debug("object_destroyed");
	ck::registry::created::remove_by_ptr(object);
}

namespace fallout {
    int scriptRemove(int index);
}
namespace ck::common {
    static const char* SYSTEM_MOD_ID = "__ck_system__";
    const char* system_mod_id() {
        return SYSTEM_MOD_ID;
    }

    const char* current_mod_id() {
        return ck::dispatcher::current_mod_context();
    }

    unsigned int current_combat_state() { return fallout::gCombatState; }
    bool currently_in_combat() { return (current_combat_state() & fallout::COMBAT_STATE_IN_COMBAT) != 0; }

    // loadsave.cc
    void on_before_game_save() {
        logger.debug("on_before_game_save");

        ck::registry::created::remove_scripts();
        ck::registry::deleted::unhide();
        ck::registry::modified::restore_sids();
        ck::proto::item::sync_custom_items_on_map(ck::proto::SyncMode::Prepare);
    }

    void on_game_save(const char* path) {
        logger.debug("on_game_save");

        ck_state_save(path);

        ck::registry::created::restore_scripts();
        ck::registry::deleted::hide();
        ck::registry::modified::reapply_sids();
        ck::proto::item::sync_custom_items_on_map(ck::proto::SyncMode::Restore);
    }

    void on_before_game_load(const char* path) {
        logger.debug("ck_scripting_on_before_game_load");

        ck::registry::clear();
        ck_state_load(path);
    }

    void on_game_loaded() {
        ck::dispatcher::on_game_loaded();
        ck::on_map_enter();
    }
}

// ffi

void ck_scripting_load_game_slot(int slot) {
	fallout::ck_load_game_slot(slot);
}

bool ck_object_float_msg(void* ptr, const char* text, int msg_type) {
	if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);
    if (!object) return false;
    if (object->elevation != fallout::gElevation) return false;

	int color = fallout::_colorTable[32747], background_color = fallout::_colorTable[0], font = 101;

	switch (msg_type) {
		case 1: // (FLOATING_MESSAGE_TYPE_WHITE)
			color = fallout::_colorTable[32767];
			break;
		case 2: // (FLOATING_MESSAGE_TYPE_RED)
			color = fallout::_colorTable[31744];
			break;
		case 3: // (FLOATING_MESSAGE_TYPE_GREEN)
			color = fallout::_colorTable[992];
			break;
		case 4: // (FLOATING_MESSAGE_TYPE_BLUE)
			color = fallout::_colorTable[31];
			break;
		default: // YELLOW
			color = fallout::_colorTable[32747];
			break;
	}

	fallout::Rect rect;
	std::string converted = utf8_to_cp1251(std::string_view(text));

	std::vector<char> safe_buffer(converted.size() + 5, '\0');
	std::memcpy(safe_buffer.data(), converted.c_str(), converted.size());

	char* safe_text_ptr = safe_buffer.data();

	if (fallout::textObjectAdd(object, safe_text_ptr, font,
                static_cast<fallout::Color>(color), static_cast<fallout::Color>(background_color), &rect) != -1) {
		fallout::tileWindowRefreshRect(&rect, object->elevation);

        return true;
	}

    return false;
}

const char* ck_testing_get_current_suite() { return g_test_suite_name.c_str(); }
void ck_testing_set_current_suite(const char* name) { g_test_suite_name = std::string(name); }
void ck_scripting_monitor_print_message(const char* message) { ck_print_monitor_message(message); }
void ck_sound_play_sfx(const char* name) { if (name != nullptr) fallout::soundPlayFile(name); }
bool ck_in_combat() { return ck::common::currently_in_combat(); }
bool ck_mods_reload_in_progress() { return ck_reloading_mods(); }
const char* ck_mods_system_id() { return ck::common::system_mod_id(); }
