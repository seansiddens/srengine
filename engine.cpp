#include "engine.h"

#include "log.h"

namespace sren {

bool Engine::init() {
    if (!init_window()) {
        LOG_ERR("Failed to initialize window!");
        return false;
    }
    LOG_INFO("Initialized window.");

    return true;
}

bool Engine::init_window() { return true; }

void Engine::shutdown() {}

} // namespace sren