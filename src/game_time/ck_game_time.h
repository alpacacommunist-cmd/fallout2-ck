// src/game_time/ck_game_time.h
#ifndef CK_SCRIPTING_GAME_TIME_H
#define CK_SCRIPTING_GAME_TIME_H

#include "ck_api.h"

// ffi
CK_API int ck_game_time_get_year();
CK_API int ck_game_time_get_day();
CK_API int ck_game_time_get_month();
CK_API int ck_game_time_get_hour();
CK_API int ck_game_time_get_time();

#endif
