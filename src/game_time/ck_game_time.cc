#include "game_time/ck_game_time.h"

namespace fallout {
    unsigned int gameTimeGetTime();
    void gameTimeGetDate(int* monthPtr, int* dayPtr, int* yearPtr);
    int gameTimeGetHour();
}

// ffi
int ck_game_time_get_year() {
	int year = 0;
	fallout::gameTimeGetDate(nullptr, nullptr, &year);
	return year;
}

int ck_game_time_get_day() {
	int day = 0;
	fallout::gameTimeGetDate(nullptr, &day, nullptr);
	return day;
}

int ck_game_time_get_month() {
	int month = 0;
	fallout::gameTimeGetDate(&month, nullptr, nullptr);
	return month;
}

int ck_game_time_get_hour() {
	return fallout::gameTimeGetHour();
}

int ck_game_time_get_time() {
	return fallout::gameTimeGetTime();
}
