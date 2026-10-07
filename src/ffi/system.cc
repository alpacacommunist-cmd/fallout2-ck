#include "ffi/system.h"
#include "ck_mods.h"
#include "ck_dispatcher/ck_dispatcher.h"

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
    return ck::dispatcher::current_context();
}
