#include "ck_log.h"
#include <iostream>

void Logger::do_log_formatted(std::string_view level, std::string_view color,
                              std::string_view fmt, std::format_args args) const {
    std::string msg = std::vformat(fmt, args);

    if (level == "ERROR") {
        print_error_log(msg);
    } else if (level == "DEBUG") {
        print_debug_log(msg);
    } else if (level == "RAW") {
        raw(msg);
    } else {
        print_log(level, color, msg);
    }
}

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

void Logger::raw(std::string_view message) const {
    std::cout << message << "\n";
}

void Logger::print_header_log(LogStyle style, std::string_view title) const {
    const size_t total_width = 80;

    std::string prefix_part = std::format("\033[1\033[37m[{}]\033[0m", m_prefix);
    size_t clean_prefix_len = m_prefix.length() + 2;

    if (style == LogStyle::Minimal) {
        // ───
        std::string left_pad = "─── ";
        std::string mid_part = std::format("{} ─── ", prefix_part);
        std::string right_part = std::format("{} ", title);

        size_t used_len = 4 + clean_prefix_len + 5 + right_part.length();
        std::string fill_line = "";
        if (total_width > used_len) {
            fill_line = std::string(total_width - used_len, '-');
        }

        size_t fill_chars = (total_width > used_len) ? (total_width - used_len) : 0;
        std::string utf8_fill = "";
        for(size_t i = 0; i < fill_chars; ++i) utf8_fill += "─";

        std::cout << "\033[90m───\033[0m " << mid_part << "\033[1m\033[37m" << title << "\033[0m \033[90m" << utf8_fill << "\033[0m\n";
    }
    else if (style == LogStyle::Cyberpunk) {
        // ▒▒▒
        std::string mid_part = std::format("{} ▒▒▒ ", prefix_part);
        size_t used_len = 4 + clean_prefix_len + 5 + title.length() + 1;

        size_t fill_chars = (total_width > used_len) ? (total_width - used_len) / 3 : 0; // '▒' takes 3 byte UTF-8
        std::string utf8_fill = "";
        for(size_t i = 0; i < fill_chars; ++i) utf8_fill += "▒";

        std::cout << "\033[36m▒▒▒\033[0m " << mid_part << "\033[1m\033[36m" << title << "\033[0m \033[36m" << utf8_fill << "\033[0m\n";
    }
    else if (style == LogStyle::Modern) {
        // ❯❯❯
        std::cout << "\033[35m❯❯❯\033[0m " << prefix_part << " \033[35m❯\033[0m \033[1m" << title << "\033[0m\n";
    }
}

