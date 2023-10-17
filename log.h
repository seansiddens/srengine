#pragma once

#include <iostream>
#include <memory>
#include <string>

#define ENABLE_DEBUG_LOGGING

#define LOG_INFO(format, ...)                                                                 \
    sren::LogService::log_info(__FILE__, __LINE__, format, ##__VA_ARGS__);
#define LOG_ERR(format, ...)                                                                  \
    sren::LogService::log_error(__FILE__, __LINE__, format, ##__VA_ARGS__);
#ifdef ENABLE_DEBUG_LOGGING
#define LOG_DBG(format, ...)                                                                  \
    sren::LogService::log_debug(__FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_DBG(format, ...)
#endif

namespace sren {

class LogService {
  public:
    template <typename... Args>
    static void log_info(const char *file, int line, const char *format, Args... args) {
        log("\033[1;32mINFO\033[0m", file, line, format, args...); // Green
    }

    template <typename... Args>
    static void log_error(const char *file, int line, const char *format, Args... args) {
        log("\033[1;31mERROR\033[0m", file, line, format, args...); // Red
    }

    template <typename... Args>
    static void log_debug(const char *file, int line, const char *format, Args... args) {
        log("\033[1;34mDEBUG\033[0m", file, line, format, args...); // Blue
    }

  private:
    template <typename... Args>
    static void log(const char *level, const char *file, int line, const char *format,
                    Args... args) {
        std::string message = string_format(format, args...);
        std::cout << "[" << level << "][" << file << ":" << line << "]: " << message
                  << std::endl;
    }

    template <typename... Args>
    static std::string string_format(const char *format, Args... args) {
        size_t size = snprintf(nullptr, 0, format, args...) + 1; // +1 for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format, args...);
        return std::string(buf.get(), buf.get() + size - 1); // -1 for '\0'
    }
};

} // namespace sren
