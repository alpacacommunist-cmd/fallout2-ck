#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

void ck_reload_mods();
void ck_scripting_init();
void ck_scripting_exit();

void ck_scripting_on_game_start();
void ck_scripting_on_day_passed();
void ck_scripting_on_game_loaded();
void ck_scripting_on_map_enter();

void ck_scripting_on_after_rest(int hours, int minutes);
void ck_scripting_on_time_advance(int hours, int minutes);

int ck_get_config_int(const char* key, int default_value);

#endif // CK_SCRIPTING_H
