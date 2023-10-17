#pragma once

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
};

} // namespace sren