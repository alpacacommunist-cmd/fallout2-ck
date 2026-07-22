#pragma once
#include <string>
#include <string_view>
#include <format>

class Logger {
private:
    std::string m_prefix;

public:
    void print_log(std::string_view tag, std::string_view tag_color, std::string_view message) const;
    void print_error_log(std::string_view message) const;
    void print_debug_log(std::string_view message) const;
	void raw(std::string_view message) const;

    explicit Logger(std::string_view prefix) : m_prefix(prefix) {}

    inline static bool debug_enabled = true;

    template <typename... Args>
    void info(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        print_log("INFO", "\033[32m", msg);
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        print_log("WARN", "\033[33m", msg);
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        print_error_log(msg);
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt_str, Args&&... args) const {
        if (!debug_enabled) return;
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        print_debug_log(msg);
    }

    template <typename... Args>
    void raw(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        raw(msg);
    }

};
