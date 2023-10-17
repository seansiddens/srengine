#pragma once

#include <iostream>
#include <string>

#define LOG_INFO(msg) sren::LogService::log_info(msg);
#define LOG_ERR(msg) sren::LogService::log_error(msg);
#define LOG_DBG(msg) sren::LogService::log_debug(msg);

namespace sren {

class LogService {
public:
    static void log_info(const std::string& message) {
        log("INFO", message);
    }

    static void log_error(const std::string& message) {
        log("ERROR", message);
    }

    static void log_debug(const std::string& message) {
        log("DEBUG", message);
    }

private:
    static void log(const std::string& level, const std::string& message) {
        std::cout << "[" << level << "]: " << message << std::endl;
    }
};


} // namespace sren