#ifndef CK_INIT_H
#define CK_INIT_H

namespace ck::init {
    bool test_mode_enabled();
    std::string& current_test_suite_name();
    void set_test_suite(const char* name);
    void set_language();
}

#endif
