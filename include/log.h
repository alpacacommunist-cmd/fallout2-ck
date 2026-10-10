#pragma once

#include <format>
#include <string_view>

namespace logging {

// ─── Levels ───────────────────────────────────────────────

enum class Level {
    Info,
    Success,
    Debug,
    Warning,
    Error,
};

namespace levels {
    inline constexpr Level info    = Level::Info;
    inline constexpr Level success = Level::Success;
    inline constexpr Level debug   = Level::Debug;
    inline constexpr Level warning = Level::Warning;
    inline constexpr Level error   = Level::Error;
}

// ─── Appearance ───────────────────────────────────────────

enum class HeaderStyle {
    Rule,       // ─── [context] ─── Title ───────────────
    Minimal,    // [context] Title
    Banner,     // ═══ Title ═════════════════════════════
};

// Helps distinguish Lua and C++ messages in the console.
enum class Source {
    Native,
    Lua,
};

// ─── Internal implementation ──────────────────────────────

namespace detail {

void write(Level level, Source source,
           std::string_view context,
           std::string_view message);

void write_header(Level level, Source source,
                  std::string_view context,
                  HeaderStyle style,
                  std::string_view title);

void write_raw(std::string_view message);

template <typename... Args>
std::string format(std::format_string<Args...> fmt,
                   Args&&... args)
{
    return std::vformat(
        fmt.get(),
        std::make_format_args(args...)
    );
}

} // namespace detail

// ─── Context-free API ─────────────────────────────────────

template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args)
{
    detail::write(Level::Info, Source::Native, {},
                  detail::format(fmt, args...));
}

template <typename... Args>
void success(std::format_string<Args...> fmt, Args&&... args)
{
    detail::write(Level::Success, Source::Native, {},
                  detail::format(fmt, args...));
}

template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args)
{
    detail::write(Level::Debug, Source::Native, {},
                  detail::format(fmt, args...));
}

template <typename... Args>
void warning(std::format_string<Args...> fmt, Args&&... args)
{
    detail::write(Level::Warning, Source::Native, {},
                  detail::format(fmt, args...));
}

template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args)
{
    detail::write(Level::Error, Source::Native, {},
                  detail::format(fmt, args...));
}

template <typename... Args>
void header(Level level, std::format_string<Args...> fmt,
            Args&&... args)
{
    detail::write_header(
        level, Source::Native, {},
        HeaderStyle::Rule,
        detail::format(fmt, args...)
    );
}

template <typename... Args>
void header(Level level, HeaderStyle style, std::format_string<Args...> fmt, Args &&...args) {
    detail::write_header(level, Source::Native, {}, style, detail::format(fmt, args...));
}

inline void raw(std::string_view message) { detail::write_raw(message); }

// ─── Optional context wrapper ──────────────────────────────

struct Context {
    std::string_view name;
    Source source = Source::Native;

    template <typename... Args> void info(std::format_string<Args...> fmt, Args &&...args) const {
        detail::write(Level::Info, source, name, detail::format(fmt, args...));
    }

    template <typename... Args> void success(std::format_string<Args...> fmt, Args &&...args) const {
        detail::write(Level::Success, source, name, detail::format(fmt, args...));
    }

    template <typename... Args> void debug(std::format_string<Args...> fmt, Args &&...args) const {
        detail::write(Level::Debug, source, name, detail::format(fmt, args...));
    }

    template <typename... Args> void warning(std::format_string<Args...> fmt, Args &&...args) const {
        detail::write(Level::Warning, source, name, detail::format(fmt, args...));
    }

    template <typename... Args> void error(std::format_string<Args...> fmt, Args &&...args) const {
        detail::write(Level::Error, source, name, detail::format(fmt, args...));
    }

    template <typename... Args> void header(Level level, std::format_string<Args...> fmt, Args &&...args) const {
        detail::write_header(level, source, name, HeaderStyle::Rule, detail::format(fmt, args...));
    }

    template <typename... Args>
    void header(Level level, HeaderStyle style, std::format_string<Args...> fmt, Args &&...args) const {
        detail::write_header(level, source, name, style, detail::format(fmt, args...));
    }
};

} // namespace logging
