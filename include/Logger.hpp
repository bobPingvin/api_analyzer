#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    static Logger& instance();
    void log(const std::string& level, const std::string& message);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

private:
    Logger();
    ~Logger();
    std::ofstream logFile_;
    std::mutex logMutex_;
};