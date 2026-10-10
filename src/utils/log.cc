#include "log.h"
#include <iostream>

namespace logging::detail {
namespace {

constexpr std::string_view reset   = "\033[0m";
constexpr std::string_view bold    = "\033[1m";
constexpr std::string_view dim     = "\033[2m";

constexpr std::string_view white   = "\033[37m";
constexpr std::string_view gray    = "\033[90m";
constexpr std::string_view cyan    = "\033[36m";
constexpr std::string_view green   = "\033[32m";
constexpr std::string_view yellow  = "\033[33m";
constexpr std::string_view red     = "\033[31m";
constexpr std::string_view magenta = "\033[35m";

struct LevelStyle {
    std::string_view label;
    std::string_view color;
};

LevelStyle style_for(Level level) {
    switch (level) {
    case Level::Info:    return {"INFO",    cyan};
    case Level::Success: return {"OK",      green};
    case Level::Debug:   return {"DEBUG",   gray};
    case Level::Warning: return {"WARN",    yellow};
    case Level::Error:   return {"ERROR",   red};
    }
    return {"INFO", cyan};
}

std::string_view source_label(Source source) {
    switch (source) {
    case Source::Native: return "C++";
    case Source::Lua:    return "LUA";
    }
    return "?";
}

std::string_view source_color(Source source) {
    switch (source) {
    case Source::Native: return cyan;
    case Source::Lua:    return magenta;
    }
    return white;
}

void write_context(Source source, std::string_view context) {
    std::cout << dim << source_color(source) << '[' << source_label(source) << ']' << reset;

    if (!context.empty()) {
        std::cout << ' ' << bold << white << '[' << context << ']' << reset;
    }
}

} // namespace

void do_log(Level level, Source source, Style style, std::string_view context, std::string_view fmt, std::format_args args) {
    std::string message = std::vformat(fmt, args);
    const auto lvl_style = style_for(level);

    switch (style) {
    case Style::Raw:
        std::cout << message << '\n';
        break;
    case Style::Normal:
        write_context(source, context);
        std::cout << ' ' << bold << lvl_style.color << '[' << lvl_style.label << ']' << reset
                  << ' ' << lvl_style.color << message << reset << '\n';
        break;

    case Style::Rule:
        write_context(source, context);
        std::cout << ' ' << lvl_style.color << "─── " << bold << message << reset
                  << ' ' << lvl_style.color << "────────────────────────────────" << reset << '\n';
        break;

    case Style::Minimal:
        write_context(source, context);
        std::cout << ' ' << lvl_style.color << lvl_style.label << reset
                  << ' ' << dim << message << reset << '\n';
        break;

    case Style::Banner:
        std::cout << lvl_style.color << "══════════════════════════════════════" << reset << '\n'
                  << bold << lvl_style.color << "  " << message << reset << '\n'
                  << lvl_style.color << "══════════════════════════════════════" << reset << '\n';
        break;
    }
}

} // namespace logging::detail
