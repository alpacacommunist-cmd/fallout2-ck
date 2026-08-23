#include "ck_utils.h"
#include <algorithm>
#include <cstring>

namespace ck::utils {
    void copy_to_buffer(char* dest, size_t max_size, std::string_view source) {
        if (max_size == 0 || dest == nullptr) return;

        size_t copy_len = std::min(source.size(), max_size - 1);
        
        std::memcpy(dest, source.data(), copy_len);
        dest[copy_len] = '\0';
    }
}
