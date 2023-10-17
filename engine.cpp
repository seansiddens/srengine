#include "engine.h"

#include "log.h"

namespace sren {

bool Engine::init() {
    if (!window.init(800, 600, "Sren Engine")) {
        LOG_ERR("Failed to initialize window!");
        return false;
    }
    LOG_INFO("Initialized window.");

    LOG_INFO("Engine succesfully initialized.");
    return true;
}

void Engine::shutdown() {
    window.teardown();
    LOG_INFO("Engine shutdown.");
}

void Engine::run() {
    while (!window.requested_exit) {
        window.handle_os_messages();
    }
}

} // namespace sren