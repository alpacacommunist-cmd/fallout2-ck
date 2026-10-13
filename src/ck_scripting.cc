#include "ck_scripting.h"
#include "ck_mods/ck_mods.h"
#include "ck_gameplay_common.h"

#include "ck_registry/ck_registry.h"
#include "ck_i18n/ck_i18n.h"
#include "ck_lua_proxy/ck_lua_proxy.h"

#include "settings.h"

#include "log.h"
static logging::Context logger{"ck_scripting.cc", logging::Source::Native};

namespace fallout {
    // loadsave.cc
	int ck_load_game_slot(int slot);
}

namespace ck {
    // ck_critter.cc
    namespace critter::proto {
        void clear_prototypes_for_mod(const char* mod_id);
    }
    // map/ck_map_camera_borders.cc
    namespace map::borders {
        void clear_for_mod(const char* mod_id);
    }
    // ck_rendering.cc
    namespace rendering {
        void clear_for_mod(const char* mod_id);
    }
    // ck_lua_proxy.cc
    namespace proxy::detail {
        extern int bootstrap;
        extern int set_language;
        extern int clear_registries;
        extern int map_exit;
    }
}

namespace ck::common {
    void clear_lua_registries() {
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::clear_registries);
    }

    void lua_map_exit() {
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::map_exit);
    }
}

// TODO: move to mods.cc
void ck_clear_mod_resources(const char* mod_id) {
    const char* mod_id_ptr = ck::mods::ptr(mod_id);

    ck::registry::clear_resources_for_mod(mod_id_ptr);
    ck::critter::proto::clear_prototypes_for_mod(mod_id_ptr);
    ck::map::borders::clear_for_mod(mod_id_ptr);
    ck::rendering::clear_for_mod(mod_id_ptr);
}

void ck_set_language() {
    logger.warning.banner("System language: {}", fallout::settings.system.language);
    ck::i18n::load_language(fallout::settings.system.language);
    ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::set_language, fallout::settings.system.language);
}

// FFI
void ck_load_game_slot(int slot) {
	fallout::ck_load_game_slot(slot);
}


// FFI

void ck_monitor_print_message(const char* message) { ck::common::print_monitor_message(message); }
void ck_sound_play_sfx(const char* name) { if (name != nullptr) ck::common::play_sfx(name); }
bool ck_in_combat() { return ck::common::in_combat(); }
bool ck_game_is_loading() { return ck::common::game_is_loading(); }
int ck_loading_map_id() { return ck::common::loading_map_id(); }
int ck_current_map_id() { return ck::common::current_map_id(); }
int ck_current_elevation() { return ck::common::current_elevation(); }

// TODO: move to mods.cc
void ck_mods_clear_resources_for_mod(const char* mod_id) {
    ck_clear_mod_resources(mod_id);
}
