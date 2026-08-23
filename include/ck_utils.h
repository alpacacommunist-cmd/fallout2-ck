#pragma once
#include <string_view>

namespace ck::utils {
    void copy_to_buffer(char* dest, size_t max_size, std::string_view source);
}

