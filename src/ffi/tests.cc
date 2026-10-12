#include "ffi/tests.h"
#include "ck_events/ck_events.h"

bool ck_testing_get_current_suite() {
    return ck::events::current_test_suite_name().data();
}

void ck_testing_set_current_suite(const char* name) {
    return ck::events::set_test_suite(name);
}
