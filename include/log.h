#pragma once

#include <format>
#include <string_view>

namespace logging {

enum class Level { Info, Success, Debug, Warning, Error };
enum class Style { Normal, Rule, Minimal, Banner, Raw };
enum class Source { Native, Lua };

namespace detail {
    void do_log(Level level, Source source, Style style, std::string_view context, std::string_view fmt, std::format_args args);
}

struct Logger {
    Level level;
    Source source = Source::Native;
    std::string_view context = "";
    Style style = Style::Normal;

    template <typename... Args>
    void operator()(std::format_string<Args...> fmt, Args&&... args) const {
        detail::do_log(level, source, style, context, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void banner(std::format_string<Args...> fmt, Args&&... args) const {
        detail::do_log(level, source, Style::Banner, context, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void rule(std::format_string<Args...> fmt, Args&&... args) const {
        detail::do_log(level, source, Style::Rule, context, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void minimal(std::format_string<Args...> fmt, Args&&... args) const {
        detail::do_log(level, source, Style::Minimal, context, fmt.get(), std::make_format_args(args...));
    }
};

struct Context {
    Logger info;
    Logger success;
    Logger debug;
    Logger warning;
    Logger error;

    Logger raw_logger;

    constexpr Context(std::string_view name, Source source = Source::Native)
        : info   { Level::Info,    source, name }
        , success{ Level::Success, source, name }
        , debug  { Level::Debug,   source, name }
        , warning{ Level::Warning, source, name }
        , error  { Level::Error,   source, name }
        , raw_logger{ Level::Info,    source, "",   Style::Raw }
    {}

    template <typename... Args>
    void raw(std::format_string<Args...> fmt, Args&&... args) const {
        raw_logger(fmt, std::forward<Args>(args)...);
    }
};

// no context log
inline constexpr Logger info   { Level::Info };
inline constexpr Logger success{ Level::Success };
inline constexpr Logger debug  { Level::Debug };
inline constexpr Logger warning{ Level::Warning };
inline constexpr Logger error  { Level::Error };
inline constexpr Logger raw{ Level::Info, Source::Native, "", Style::Raw };

} // namespace logging
