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
inline constexpr Level info = Level::Info;
inline constexpr Level success = Level::Success;
inline constexpr Level debug = Level::Debug;
inline constexpr Level warning = Level::Warning;
inline constexpr Level error = Level::Error;
} // namespace levels

// ─── Appearance ───────────────────────────────────────────

enum class HeaderStyle {
    Rule,    // ─── [context] ─── Title ───────────────
    Minimal, // [context] Title
    Banner,  // ═══ Title ═════════════════════════════
};

// Helps distinguish Lua and C++ messages in the console.
enum class Source {
    Native,
    Lua,
};

// ─── Internal implementation ──────────────────────────────

namespace detail {
void log_impl(Level level, Source source, std::string_view name, std::string_view fmt, std::format_args args);
void log_header_impl(Level level, Source source, std::string_view context,
        HeaderStyle style, std::string_view fmt, std::format_args args);
void log_raw_impl(std::string_view fmt, std::format_args args);
} // namespace detail

// ─── Context-free API ─────────────────────────────────────

template <typename... Args> void info(std::format_string<Args...> fmt, Args... args) {
    detail::log_impl(Level::Info, Source::Native, {}, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void success(std::format_string<Args...> fmt, Args... args) {
    detail::log_impl(Level::Success, Source::Native, {}, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void debug(std::format_string<Args...> fmt, Args... args) {
    detail::log_impl(Level::Debug, Source::Native, {}, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void warning(std::format_string<Args...> fmt, Args... args) {
    detail::log_impl(Level::Warning, Source::Native, {}, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void error(std::format_string<Args...> fmt, Args... args) {
    detail::log_impl(Level::Error, Source::Native, {}, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void header(Level level, std::format_string<Args...> fmt, Args... args) {
    detail::log_header_impl(level, Source::Native, {}, HeaderStyle::Rule, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void header(Level level, HeaderStyle style, std::format_string<Args...> fmt, Args... args) {
    detail::log_header_impl(level, Source::Native, {}, style, fmt.get(), std::make_format_args(args...));
}

template <typename... Args> void raw(std::format_string<Args...> fmt, Args... args) {
    detail::log_raw_impl(fmt.get(), std::make_format_args(args...));
}

// inline void raw(std::string_view message) { detail::write_raw(message); }

// ─── Optional context wrapper ──────────────────────────────

struct Context {
    std::string_view name;
    Source source = Source::Native;

    template <typename... Args> void raw(std::format_string<Args...> fmt, Args... args) const {
        detail::log_raw_impl(fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args> void info(std::format_string<Args...> fmt, Args... args) const {
        detail::log_impl(Level::Info, source, name, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args> void success(std::format_string<Args...> fmt, Args... args) const {
        detail::log_impl(Level::Success, source, name, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args> void debug(std::format_string<Args...> fmt, Args... args) const {
        detail::log_impl(Level::Debug, source, name, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args> void warning(std::format_string<Args...> fmt, Args... args) const {
        detail::log_impl(Level::Warning, source, name, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args> void error(std::format_string<Args...> fmt, Args... args) const {
        detail::log_impl(Level::Error, source, name, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args> void header(Level level, std::format_string<Args...> fmt, Args... args) const {
        detail::log_header_impl(level, source, name, HeaderStyle::Rule, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    void header(Level level, HeaderStyle style, std::format_string<Args...> fmt, Args... args) const {
        detail::log_header_impl(level, source, name, style, fmt.get(), std::make_format_args(args...));
    }
};

} // namespace logging
