#include "ffi/gameplay.h"
#include "ck_gameplay_common.h"

bool ck_object_float_msg(void *ptr, const char *text, int msg_type) {
    return ck::common::object_float_msg(ptr, text, msg_type);
}

void ck_monitor_print_message(const char *message) {
    ck::common::print_monitor_message(message);
}

void ck_sound_play_sfx(const char *name) {
    if (name != nullptr) ck::common::play_sfx(name);
}

bool ck_in_combat() {
    return ck::common::in_combat();
}

bool ck_game_is_loading() {
    return ck::common::game_is_loading();
}

int ck_loading_map_id() {
    return ck::common::loading_map_id();
}

int ck_current_map_id() {
    return ck::common::current_map_id();
}

int ck_current_elevation() {
    return ck::common::current_elevation();
}
