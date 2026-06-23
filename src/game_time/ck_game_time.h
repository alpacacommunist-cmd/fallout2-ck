// src/game_time/ck_game_time.h
#ifndef CK_SCRIPTING_GAME_TIME_H
#define CK_SCRIPTING_GAME_TIME_H

#include "ck_api.h"

namespace ck {
	void on_day_passed();
	void on_after_rest(int hours, int minutes);
	void on_time_advance(int hours, int minutes);
}

// ffi
CK_API int ck_game_get_year();
CK_API int ck_game_get_day();
CK_API int ck_game_get_month();
CK_API int ck_game_get_hour();
CK_API int ck_game_get_time();

#endif
