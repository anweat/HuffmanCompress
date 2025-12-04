#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

Logger::Logger() : currentLogLevel(LogLevel::DEBUG) {
    logFileName = "application.log";
    logFile.open(logFileName, std::ios::out | std::ios::app);
    
#ifdef _WIN32
    // 在Windows上设置控制台支持Unicode
    if (isConsoleSupportsUnicode()) {
        system("chcp 65001 > nul");  // 设置代码页为UTF-8
    }
#endif
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
    logFileName = filename;
    logFile.open(logFileName, std::ios::out | std::ios::app);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentLogLevel = level;
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < currentLogLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = getCurrentTime();
    std::string levelStr = logLevelToString(level);
    
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";
    
    if (logFile.is_open()) {
        logFile << logMessage;
        logFile.flush();
    }
    
    // 同时输出到控制台
    std::cout << logMessage;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

// 异常捕获和记录函数实现
void Logger::logException(const std::exception& e) {
    log(LogLevel::ERROR, "Exception caught: " + std::string(e.what()));
}

void Logger::logException(const std::exception& e, const std::string& context) {
    log(LogLevel::ERROR, "Exception caught in " + context + ": " + std::string(e.what()));
}

#ifdef _WIN32
bool Logger::isConsoleSupportsUnicode() {
    // 检查是否在控制台环境中
    return _isatty(_fileno(stdout)) != 0;
}
#endif
