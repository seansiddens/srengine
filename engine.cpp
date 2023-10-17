#include "engine.h"

#include "log.h"

namespace sren {

const u32 window_width = 800;
const u32 window_height = 600;

bool Engine::init() {
    // Initialize window.
    if (!window.init(window_width, window_height, "Sren Engine")) {
        LOG_ERR("Failed to initialize window!");
        return false;
    }

    // Initialize device.
    if (!device.init(window_width, window_height, window.window_handle)) {
        LOG_ERR("Failed to initialize device!");
        return false;
    }

    LOG_INFO("Engine succesfully initialized.");
    return true;
}

void Engine::shutdown() {
    // TODO: Better way of automatically cleaning everything up?
    window.teardown();
    device.teardown();
    LOG_INFO("Engine shutdown.");
}

void Engine::run() {
    while (!window.requested_exit) {
        window.handle_os_messages();
    }
}

} // namespace sren