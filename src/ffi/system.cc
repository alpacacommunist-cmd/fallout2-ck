#include "ffi/system.h"
#include "ck_mods/ck_mods.h"
#include "ck_dispatcher/ck_dispatcher.h"

namespace fallout {
    // loadsave.cc
	int ck_load_game_slot(int slot);
}

bool ck_system_add_mod(const char* mod_id) {
    return ck::mods::add(mod_id);
}

bool ck_system_drop_mod(const char* mod_id) {
    return ck::mods::drop(mod_id);
}

bool ck_system_set_mod_context(const char* mod_id) {
    return ck::dispatcher::set_context(mod_id);
}

void ck_system_emit_for_mod(const char* mod_id, const char* event_name) {
    return ck::dispatcher::emit_for_mod(mod_id, event_name);
}

const char* ck_system_current_mod_id() {
    return ck::mods::current_mod_id();
}

bool ck_system_reloading_mods() {
    return ck::mods::reloading();
}

const char* ck_sytem_mod_id() {
    return ck::mods::system_mod_id();
}

void ck_load_game_slot(int slot) {
	fallout::ck_load_game_slot(slot);
}

void ck_system_log(int level, int source, int style, const char* context, const char* message) {
    logging::detail::render_log(
            static_cast<logging::Level>(level),
            static_cast<logging::Source>(source),
            static_cast<logging::Style>(style),
            context ? context : "",
            message ? message : ""
            );
}
