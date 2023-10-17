#pragma once

#include "platform.h"
#include "vk_common.h"

#include <SDL2/SDL.h>

namespace sren {

class Device {
  public:
    bool init(u32 window_width, u32 window_height, SDL_Window *window);
    void teardown();

  private:
    bool get_family_queue(VkPhysicalDevice physical_device);

    VkInstance vk_instance;
    VkPhysicalDevice vk_physical_device;
    VkPhysicalDeviceProperties vk_physical_device_properties;
    VkQueue vk_queue;
    u32 vk_queue_family;

    SDL_Window *window_handle;
    VkSurfaceKHR vk_surface;

    u32 swapchain_width = 1;
    u32 swapchain_height = 1;

    bool debug_utils_extension_present = false;
    VkDebugUtilsMessengerEXT vk_debug_utils_messenger;

    // NOTE: idk know what this is used for, but I have this handle in case I want to utilize
    // it later.
    VkAllocationCallbacks *vk_alloc_callbacks = nullptr;
};

} // namespace sren