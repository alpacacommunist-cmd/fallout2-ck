// src/game_time/ck_game_time.h
#ifndef CK_SCRIPTING_GAME_TIME_H
#define CK_SCRIPTING_GAME_TIME_H

#ifdef _WIN32
  #define CK_API __declspec(dllexport)
#else
  #define CK_API
#endif

void ck_scripting_on_day_passed();
void ck_scripting_on_after_rest(int hours, int minutes);
void ck_scripting_on_time_advance(int hours, int minutes);

// ffi
extern "C" {
    CK_API int ck_game_get_year();
    CK_API int ck_game_get_day();
    CK_API int ck_game_get_month();
    CK_API int ck_game_get_hour();
    CK_API int ck_game_get_time();
}

#endif
