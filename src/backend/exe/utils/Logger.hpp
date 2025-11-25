#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

/**
 * @brief Logger class using spdlog.
 *
 * Provides a logger instance for the entire application (Singleton pattern).
 * Logs to both console and file.
 */
class Logger {
   public:
    /**
     * @brief Get the logger instance.
     * @return Reference to the logger instance
     */
    static Logger& instance();

    /**
     * @brief Log info message
     * @param message The message to log
     */
    void info(const std::string& message);

    /**
     * @brief Log debug message
     * @param message The message to log
     */
    void debug(const std::string& message);

    /**
     * @brief Log trace message
     * @param message The message to log
     */
    void trace(const std::string& message);

    /**
     * @brief Log warning message
     * @param message The message to log
     */
    void warning(const std::string& message);

    /**
     * @brief Log error message
     * @param message The message to log
     */
    void error(const std::string& message);

    /**
     * @brief Log critical error message
     * @param message The message to log
     */
    void critical(const std::string& message);

    /**
     * @brief Return true if current log level is Info
     */
    bool isLevelInfo() { return (logger_->level() == spdlog::level::level_enum::info); }
    
    /**
     * @brief Return true if current log level is Debug
     */
    bool isLevelDebug() { return (logger_->level() == spdlog::level::level_enum::debug); }
    
    /**
     * @brief Return true if current log level is Trace
     */
    bool isLevelTrace() { return (logger_->level() == spdlog::level::level_enum::trace); }

    /**
     * @brief Set the log level
     */
    void setLogLevel(spdlog::level::level_enum debug_level);

    // Delete copy constructor and assignment operator (Singleton)
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

   private:
    /**
     * @brief Private constructor (Singleton)
     */
    Logger();

    /**
     * @brief Destructor
     */
    ~Logger() = default;

    std::shared_ptr<spdlog::logger> logger_;
};