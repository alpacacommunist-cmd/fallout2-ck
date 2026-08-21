#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

#include "ck_api.h"

extern "C" bool ck_in_combat();

namespace ck {
	void on_map_enter();
}

namespace ck::proxy::detail {
    extern int reload_mods;
    extern int bootstrap;
    extern int set_language;
}

namespace fallout {
    struct Object;
    struct Rect;

	extern Object* gDude;
    extern int gElevation;

    enum Color : unsigned char;

	void tileWindowRefreshRect(Rect* rect, int elevation);
    int textObjectAdd(Object* object, char* string, int font, Color color, Color outlineColor, Rect* rect);

	void tileWindowRefresh();
	void animationStop();
	void displayMonitorAddMessage(const char* str);

	void mainMenuWindowFree();
    int soundPlayFile(const char* name);

	int  ck_load_game_slot(int slot);
}

void ck_map_clear_camera_borders();

void ck_reload_mods();
void ck_scripting_init(int argc, char** argv);
void ck_scripting_exit();

void ck_scripting_on_object_destroyed(fallout::Object* object);

void ck_scripting_on_game_start();
void ck_scripting_on_engine_ready();
void ck_scripting_on_before_game_load(const char* path);
void ck_scripting_on_game_loaded();
void ck_on_scripts_reset();
// state
void ck_scripting_on_before_game_save();
void ck_scripting_on_game_save(const char* path);

CK_API void ck_registry_destroy_objects_for_mod(const char* target_mod_id);
CK_API void ck_scripting_load_game_slot(int slot);

CK_API const char* ck_testing_get_current_suite();
CK_API void ck_testing_set_current_suite(const char* name);
CK_API void ck_scripting_monitor_print_message(const char* message);
CK_API void ck_sound_play_sfx(const char* name);
CK_API bool ck_object_float_msg(void* ptr, const char* text, int msg_type = 1);

#endif // CK_SCRIPTING_H
