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
    if (mod_id == nullptr) {
        ck::dispatcher::set_context(nullptr);
        return true;
    }

    const char* mod_id_ptr = ck::mods::ptr(mod_id);
    if (mod_id_ptr != nullptr) {
        ck::dispatcher::set_context(mod_id_ptr);
        return true;
    }

    return false;
}

void ck_system_emit_for_mod(const char* mod_id, const char* event_name) {
    return ck::dispatcher::emit_for_mod(mod_id, event_name);
}
