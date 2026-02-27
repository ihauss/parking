#pragma once

#include <iostream>
#include <string>

enum class LogLevel {
    INFO = 0,
    WARN = 1,
    ERROR = 2
};

class Logger {
private:
    LogLevel _level;
public:

    static Logger& log() {
        static Logger instance;
        return instance;
    }

    // Default constructor: INFO and above
    Logger() : _level(LogLevel::INFO) {}

    // Constructor with explicit log level
    explicit Logger(LogLevel level) : _level(level) {}

    void info(const std::string& msg) const {
        if (_level <= LogLevel::INFO) {
            std::cout << "[INFO]  " << msg << std::endl;
        }
    }

    void warn(const std::string& msg) const {
        if (_level <= LogLevel::WARN) {
            std::cout << "[WARN]  " << msg << std::endl;
        }
    }

    void error(const std::string& msg) const {
        if (_level <= LogLevel::ERROR) {
            std::cerr << "[ERROR] " << msg << std::endl;
        }
    }
};