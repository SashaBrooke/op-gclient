#include "util/logging.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
#include <filesystem>

// Configuration constants
constexpr const char* LOG_FILE = "logs/app.log";
constexpr const char* LOGGER_NAME = "app_logger";
constexpr size_t LOG_FILE_SIZE_BYTES = 1024 * 1024 * 5;  // 5 MB
constexpr size_t LOG_FILE_COUNT = 3;  // Keep 3 rotating log files

void initialize_logging() {
    try {
        // Create logs directory if it doesn't exist
        std::filesystem::create_directories("logs");
        
        // Set error handler for spdlog itself
        spdlog::set_error_handler([](const std::string& msg) { 
            fprintf(stderr, "*** LOGGER ERROR ***: %s\n", msg.c_str());
        });

        // Create console sink with colors
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);

        // Create rotating file sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            LOG_FILE, LOG_FILE_SIZE_BYTES, LOG_FILE_COUNT);
        file_sink->set_level(spdlog::level::trace);

        // Combine sinks
        spdlog::sinks_init_list sink_list = {console_sink, file_sink};

        // Create logger with both sinks
        auto logger = std::make_shared<spdlog::logger>(LOGGER_NAME, sink_list);
        
        // Set log level (trace = most verbose, critical = least verbose)
#ifdef NDEBUG
        logger->set_level(spdlog::level::info);  // Release build
#else
        logger->set_level(spdlog::level::debug);  // Debug build
#endif
        
        // Set pattern:  [timestamp] [level] message
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        
        // Always flush error level and above immediately
        logger->flush_on(spdlog::level::err);
        
        // Set as default logger
        spdlog::set_default_logger(logger);
        
        log_info("Logging system initialized");
        log_info("Log file: {}", LOG_FILE);
        
    } catch (const spdlog::spdlog_ex& ex) {
        fprintf(stderr, "Log initialization failed: %s\n", ex.what());
    }
}

void shutdown_logging() {
    // Flush all logs before shutdown
    if (spdlog::default_logger()) {
        spdlog::default_logger()->flush();
    }
    
    // Drop all loggers and clean up
    spdlog::drop_all();
}
