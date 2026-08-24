#include "ck_utils.h"
#include <algorithm>
#include <string>
#include <cstring>
#include <cctype>

#include "ck_log.h"
static const Logger logger("CK Utils");

namespace ck::utils {
    void fatal_nullptr_crash(const char* function_name) {
        logger.error("FATAL ERROR: nullptr passed to API function: '{}'", function_name);
        logger.error("The modding environment state is corrupted. Crashing now.");

        std::abort();
    }

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
