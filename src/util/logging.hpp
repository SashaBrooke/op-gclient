#ifndef LOGGING_HPP
#define LOGGING_HPP

#include "spdlog/spdlog.h"

/**
 * Initialize logging system with console and file output.
 * Call this once at application startup.
 */
void initialize_logging();

/**
 * Shutdown logging system.
 * Call this before application exit.
 */
void shutdown_logging();

// Convenient logging macros
template<typename... Args>
inline void log_trace(spdlog::format_string_t<Args...> fmt, Args &&... args) {
    spdlog::trace(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_debug(spdlog::format_string_t<Args...> fmt, Args &&... args) {
    spdlog::debug(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_info(spdlog::format_string_t<Args...> fmt, Args &&... args) {
    spdlog::info(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_warn(spdlog::format_string_t<Args...> fmt, Args &&... args) {
    spdlog::warn(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_error(spdlog::format_string_t<Args...> fmt, Args &&... args) {
    spdlog::error(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_critical(spdlog::format_string_t<Args...> fmt, Args &&... args) {
    spdlog::critical(fmt, std::forward<Args>(args)...);
}

#endif // LOGGING_HPP
