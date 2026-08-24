#include "ck_utils.h"
#include <algorithm>
#include <string>
#include <cstring>
#include <cctype>

namespace ck::utils {
    void copy_to_buffer(char* dest, size_t max_size, std::string_view source) {
        if (max_size == 0 || dest == nullptr) return;

        size_t copy_len = std::min(source.size(), max_size - 1);
        
        std::memcpy(dest, source.data(), copy_len);
        dest[copy_len] = '\0';
    }

    bool is_blank(std::string_view str) {
        if (str.empty()) return true;

        for (char ch : str) {
            if (!std::isspace(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        return true;
    }

    bool is_blank(const std::string& str) {
        return is_blank(std::string_view(str));
    }

    bool is_blank(const char* str) {
        if (str == nullptr || str[0] == '\0') return true;
        return is_blank(std::string_view(str));
    }
}
