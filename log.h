#pragma once

#include <iostream>
#include <memory>
#include <string>

#define LOG_INFO(format, ...) sren::LogService::log_info(format, ##__VA_ARGS__);
#define LOG_ERR(format, ...) sren::LogService::log_error(format, ##__VA_ARGS__);
#define LOG_DBG(format, ...) sren::LogService::log_debug(format, ##__VA_ARGS__);

namespace sren {

class LogService {
public:
    template <typename... Args>
    static void log_info(const char* format, Args... args) {
        log("INFO", format, args...);
    }

    template <typename... Args>
    static void log_error(const char* format, Args... args) {
        log("ERROR", format, args...);
    }

    template <typename... Args>
    static void log_debug(const char* format, Args... args) {
        log("DEBUG", format, args...);
    }

private:
    template <typename... Args>
    static void log(const char* level, const char* format, Args... args) {
        std::string message = string_format(format, args...);
        std::cout << "[" << level << "]: " << message << std::endl;
    }

    template<typename ... Args>
    static std::string string_format(const char* format, Args ... args) {
        size_t size = snprintf(nullptr, 0, format, args ...) + 1; // +1 for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format, args ...);
        return std::string(buf.get(), buf.get() + size - 1); // -1 for '\0'
    }
};

} // namespace sren
