#include "ffi/tests.h"
#include "system/init.h"

const char* ck_testing_get_current_suite() {
    return ck::init::current_test_suite_name().data();
}

void ck_testing_set_current_suite(const char* name) {
    return ck::init::set_test_suite(name);
}
