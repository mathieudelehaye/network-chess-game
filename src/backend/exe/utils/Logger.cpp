#include "Logger.hpp"

#include <filesystem>
#include <vector>

Logger::Logger() {
    // change this with one of the values: `info`, `debug`, `warn`, `err`
    const auto debug_level = spdlog::level::debug;

    // Get executable path
    std::filesystem::path exe_path = std::filesystem::canonical("/proc/self/exe");

    // Project root is 3 levels up from bin/backend/chess_server
    std::filesystem::path project_root = exe_path.parent_path().parent_path().parent_path();

    // Log directory
    std::filesystem::path log_dir = project_root / "log";

    // Create log directory if it doesn't exist
    std::filesystem::create_directories(log_dir);

    // Console sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(debug_level);

    // File sink (overwrites on each run)
    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>((log_dir / "server.log").string(),
                                                            true  // truncate file
        );
    file_sink->set_level(debug_level);

    // Combine sinks
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    logger_ = std::make_shared<spdlog::logger>("ChessServer", sinks.begin(), sinks.end());

    // Set log level and pattern
    logger_->set_level(debug_level);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    // Set as default logger
    spdlog::set_default_logger(logger_);

    // Flush on info level and higher
    logger_->flush_on(spdlog::level::info);
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::info(const std::string& message) {
    logger_->info(message);
}

void Logger::debug(const std::string& message) {
    logger_->debug(message);
}

void Logger::trace(const std::string& message) {
    logger_->trace(message);
}

void Logger::warning(const std::string& message) {
    logger_->warn(message);
}

void Logger::error(const std::string& message) {
    logger_->error(message);
}