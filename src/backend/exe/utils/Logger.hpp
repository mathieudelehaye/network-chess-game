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
     * @brief Log debug message
     * @param message The message to log
     */
    void debug(const std::string& message);

    /**
     * @brief Log info message
     * @param message The message to log
     */
    void info(const std::string& message);

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