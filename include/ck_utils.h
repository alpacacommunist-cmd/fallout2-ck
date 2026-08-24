#pragma once
#include <string_view>

namespace ck::utils {
    void copy_to_buffer(char* dest, size_t max_size, std::string_view source);

    bool is_blank(std::string_view str);
    bool is_blank(const std::string& str);
    bool is_blank(const char* str);
}

