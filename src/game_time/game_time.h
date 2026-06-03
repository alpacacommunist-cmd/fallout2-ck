// src/game_time/game_time.h
#ifndef CK_SCRIPTING_GAME_TIME_BINDINGS_H
#define CK_SCRIPTING_GAME_TIME_BINDINGS_H

void ck_scripting_on_day_passed();
void ck_scripting_on_after_rest(int hours, int minutes);
void ck_scripting_on_time_advance(int hours, int minutes);

#endif
