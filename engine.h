#pragma once

#include "platform.h"

namespace sren {

class Engine {
  public:
    bool init();
    void shutdown();


  private:
    bool init_window();
    bool init_vulkan();
    bool init_resources();

    void *m_window;
};

} // namespace sren