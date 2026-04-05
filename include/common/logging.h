#pragma once

#include <string>
#include <iostream>
#include <ctime>
#include <mutex>

namespace qp {

enum class LogLevel { Info, Warning, Error };

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void set_level(LogLevel level) { min_level_ = level; }

    void info(const std::string& msg) {
        log(LogLevel::Info, msg);
    }

    void warning(const std::string& msg) {
        log(LogLevel::Warning, msg);
    }

    void error(const std::string& msg) {
        log(LogLevel::Error, msg);
    }

    static void log_info(const std::string& msg) { instance().info(msg); }
    static void log_warning(const std::string& msg) { instance().warning(msg); }
    static void log_error(const std::string& msg) { instance().error(msg); }

private:
    Logger() = default;
    LogLevel min_level_ = LogLevel::Info;
    std::mutex mutex_;

    void log(LogLevel level, const std::string& msg) {
        if (level < min_level_) return;
        std::lock_guard<std::mutex> lock(mutex_);
        std::time_t now = std::time(nullptr);
        char timebuf[64];
        std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        std::cerr << "[" << timebuf << "] " << level_str(level) << ": " << msg << "\n";
    }

    static const char* level_str(LogLevel level) {
        switch (level) {
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error:   return "ERROR";
        }
        return "UNKNOWN";
    }
};

} // namespace qp
