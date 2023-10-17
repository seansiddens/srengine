#include "engine.h"

#include "log.h"

namespace sren {

bool Engine::init() {
    if (!window.init(800, 600, "Sren Engine")) {
        LOG_ERR("Failed to initialize window!");
        return false;
    }
    LOG_INFO("Initialized window.");

    return true;
}

void Engine::shutdown() {}

} // namespace sren