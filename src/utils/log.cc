#include "log.h"

#include <iostream>

namespace logging::detail {
namespace {

constexpr std::string_view reset = "\033[0m";
constexpr std::string_view bold  = "\033[1m";
constexpr std::string_view dim   = "\033[2m";

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

LevelStyle style_for(Level level)
{
    switch (level) {
    case Level::Info:    return {"INFO",    cyan};
    case Level::Success: return {"OK",      green};
    case Level::Debug:   return {"DEBUG",   gray};
    case Level::Warning: return {"WARN",    yellow};
    case Level::Error:   return {"ERROR",   red};
    }

    return {"INFO", cyan};
}

std::string_view source_label(Source source)
{
    switch (source) {
    case Source::Native: return "C++";
    case Source::Lua:    return "LUA";
    }

    return "?";
}

std::string_view source_color(Source source)
{
    switch (source) {
    case Source::Native: return cyan;
    case Source::Lua:    return magenta;
    }

    return white;
}

void write_context(Source source, std::string_view context)
{
    std::cout << dim << source_color(source)
              << '[' << source_label(source) << ']'
              << reset;

    if (!context.empty()) {
        std::cout << ' ' << bold << white
                  << '[' << context << ']'
                  << reset;
    }
}

} // namespace

void write(Level level, Source source,
           std::string_view context,
           std::string_view message)
{
    const auto style = style_for(level);

    write_context(source, context);

    std::cout << ' '
              << bold << style.color
              << '[' << style.label << ']'
              << reset << ' '
              << style.color << message << reset
              << '\n';
}

void write_header(Level level, Source source,
                  std::string_view context,
                  HeaderStyle header_style,
                  std::string_view title)
{
    const auto style = style_for(level);

    switch (header_style) {
    case HeaderStyle::Rule:
        write_context(source, context);

        std::cout << ' '
                  << style.color << "─── "
                  << bold << title << reset
                  << ' ' << style.color
                  << "────────────────────────────────"
                  << reset << '\n';
        break;

    case HeaderStyle::Minimal:
        write_context(source, context);

        std::cout << ' '
                  << style.color << bold
                  << title << reset << '\n';
        break;

    case HeaderStyle::Banner:
        std::cout << style.color
                  << "══════════════════════════════════════"
                  << reset << '\n'
                  << bold << style.color
                  << "  " << title
                  << reset << '\n'
                  << style.color
                  << "══════════════════════════════════════"
                  << reset << '\n';
        break;
    }
}

void write_raw(std::string_view message)
{
    std::cout << message << '\n';
}

} // namespace logging::detail
