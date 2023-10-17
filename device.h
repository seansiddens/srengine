#pragma once

#include "vk_common.h"
#include "platform.h"

namespace sren {

class Device {
  public:
    bool init(u32 window_width, u32 window_height);
    void teardown();

  private:
    VkInstance instance;

    u32 swapchain_width = 1;
    u32 swapchain_height = 1;
};

} // namespace sren