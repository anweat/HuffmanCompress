#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <exception>
#include "../FileStream/sysdetect.h"

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogFile(const std::string& filename);
    void setLogLevel(LogLevel level);
    
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    // 异常捕获和记录函数
    void logException(const std::exception& e);
    void logException(const std::exception& e, const std::string& context);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string getCurrentTime();
    std::string logLevelToString(LogLevel level);
    
    std::ofstream logFile;
    std::mutex logMutex;
    LogLevel currentLogLevel;
    std::string logFileName;
    
#ifdef _WIN32
    bool isConsoleSupportsUnicode();
#endif
};