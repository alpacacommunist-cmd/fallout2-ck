#pragma once
#include <string_view>
#include <format>

class Logger {
private:
    std::string m_prefix;

    void do_log_formatted(std::string_view level, std::string_view color,
                          std::string_view fmt, std::format_args args) const;

public:
    void print_log(std::string_view tag, std::string_view tag_color, std::string_view message) const;
    void print_error_log(std::string_view message) const;
    void print_debug_log(std::string_view message) const;
    void raw(std::string_view message) const;

    explicit Logger(std::string_view prefix) : m_prefix(prefix) {}

    inline static bool debug_enabled = true;

    template <typename... Args>
    void info(std::format_string<Args...> fmt_str, Args&&... args) const {
        do_log_formatted("INFO", "\033[32m", fmt_str.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt_str, Args&&... args) const {
        do_log_formatted("WARN", "\033[33m", fmt_str.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt_str, Args&&... args) const {
        do_log_formatted("ERROR", "\033[31m", fmt_str.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt_str, Args&&... args) const {
        if (!debug_enabled) return;
        do_log_formatted("DEBUG", "\033[36m", fmt_str.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void raw(std::format_string<Args...> fmt_str, Args&&... args) const {
        do_log_formatted("RAW", "", fmt_str.get(), std::make_format_args(args...));
    }
};
