#include "ck_log.h"
#include <iostream>

void Logger::print_log(std::string_view tag, std::string_view tag_color, std::string_view message) const {
    std::cout << "\033[1m\033[37m[" << m_prefix << "]\033[0m "
              << tag_color << "[" << tag << "]\033[0m "
              << message << "\n";
}

void Logger::print_error_log(std::string_view message) const {
    std::cout << "\033[1m\033[37m[" << m_prefix << "]\033[0m "
              << "\033[1m\033[31m[ERROR]\033[0m "
              << "\033[31m" << message << "\033[0m\n";
}

void Logger::print_debug_log(std::string_view message) const {
    std::cout << "\033[1m\033[37m[" << m_prefix << "]\033[0m "
              << "\033[90m[DEBUG] " << message << "\033[0m\n";
}
