#pragma once

#include "device.h"
#include "platform.h"
#include "window.h"

namespace sren {

class Engine {
  public:
    bool init();
    void shutdown();

    void run();

  private:
    bool init_vulkan();
    bool init_resources();

    Window window;
    Device device;
};

} // namespace sren