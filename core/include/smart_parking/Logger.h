#pragma once

#include <iostream>
#include <string>
#include <mutex>

namespace smart_parking {

/**
 * @enum LogLevel
 * @brief Logging severity levels.
 */
enum class LogLevel {
    INFO = 0,
    WARN = 1,
    ERROR = 2
};

/**
 * @class Logger
 * @brief Simple thread-safe logger for console output.
 *
 * This logger is designed for lightweight debugging and monitoring.
 * It is thread-safe and supports runtime log level configuration.
 */
class Logger {
private:
    LogLevel _level;
    mutable std::mutex _mutex;

    Logger() : _level(LogLevel::INFO) {}

public:
    /**
     * @brief Access the global logger instance.
     */
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    static Logger& log() {
        return instance();
    }

    /**
     * @brief Set the minimum log level.
     */
    void setLevel(LogLevel level) {
        _level = level;
    }

    /**
     * @brief Check if a message should be logged.
     */
    bool shouldLog(LogLevel msgLevel) const {
        return static_cast<int>(msgLevel) >= static_cast<int>(_level);
    }

    void info(const std::string& msg) const {
        if (!shouldLog(LogLevel::INFO)) return;
        std::lock_guard<std::mutex> lock(_mutex);
        std::cout << "[INFO]  " << msg << "\n";
    }

    void warn(const std::string& msg) const {
        if (!shouldLog(LogLevel::WARN)) return;
        std::lock_guard<std::mutex> lock(_mutex);
        std::cout << "[WARN]  " << msg << "\n";
    }

    void error(const std::string& msg) const {
        if (!shouldLog(LogLevel::ERROR)) return;
        std::lock_guard<std::mutex> lock(_mutex);
        std::cerr << "[ERROR] " << msg << "\n";
    }
};

} // namespace smart_parking