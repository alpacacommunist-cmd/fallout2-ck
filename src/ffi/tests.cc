#include "ffi/tests.h"

namespace ck::events::system {
    // Tests helpers
    std::string& current_test_suite_name();
    void set_test_suite(const char* name);
}

const char* ck_testing_get_current_suite() {
    return ck::events::system::current_test_suite_name().data();
}

void ck_testing_set_current_suite(const char* name) {
    return ck::events::system::set_test_suite(name);
}
