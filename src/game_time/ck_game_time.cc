#include <iostream>

#include "ck_utils.h"
#include "game_time/ck_game_time.h"
#include "scripts.h"

namespace ck {

	void on_day_passed() {
		ck_dispatcher_on_day_passed();
	}

	// just in case
	void on_after_rest(int hours, int minutes) {
		on_time_advance(hours, minutes);
	}

	void on_time_advance(int hours, int minutes) {
		ck_dispatcher_on_time_advance(hours, minutes);
	}

}

// ffi
int ck_game_get_year() {
	int year = 0;
	fallout::gameTimeGetDate(nullptr, nullptr, &year);
	return year;
}

int ck_game_get_day() {
	int day = 0;
	fallout::gameTimeGetDate(nullptr, &day, nullptr);
	return day;
}

int ck_game_get_month() {
	int month = 0;
	fallout::gameTimeGetDate(&month, nullptr, nullptr);
	return month;
}

int ck_game_get_hour() {
	return fallout::gameTimeGetHour();
}

int ck_game_get_time() {
	return fallout::gameTimeGetTime();
}
