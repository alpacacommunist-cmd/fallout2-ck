#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <format>

class Logger {
private:
    std::string m_prefix;

    static constexpr std::string_view RESET  = "\033[0m";
    static constexpr std::string_view BOLD   = "\033[1m";
    static constexpr std::string_view RED    = "\033[31m";
    static constexpr std::string_view GREEN  = "\033[32m";
    static constexpr std::string_view YELLOW = "\033[33m";
    static constexpr std::string_view WHITE  = "\033[37m";
    static constexpr std::string_view GRAY   = "\033[90m";

    void print_log(std::string_view tag, std::string_view tag_color, const std::string& message) const {
        std::string fmt_prefix = std::format("{}{}[{}]{}", BOLD, WHITE, m_prefix, RESET);
        std::string fmt_tag = std::format("{}{}{}", tag_color, tag, RESET);
        std::cout << fmt_prefix << " " << fmt_tag << " " << message << std::endl;
    }

public:
    explicit Logger(std::string_view prefix) : m_prefix(prefix) {}

    inline static bool debug_enabled = true;

    template <typename... Args>
    void info(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        print_log("[INFO]", GREEN, msg);
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        print_log("[WARN]", YELLOW, msg);
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt_str, Args&&... args) const {
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        std::string colored_msg = std::format("{}{}{}", RED, msg, RESET);
        print_log("[ERROR]", std::format("{}{}", BOLD, RED), colored_msg);
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt_str, Args&&... args) const {
        if (!debug_enabled) return;
        std::string msg = std::format(fmt_str, std::forward<Args>(args)...);
        std::string colored_msg = std::format("{}{}{}", GRAY, msg, RESET);
        print_log("[DEBUG]", GRAY, colored_msg);
    }
};
