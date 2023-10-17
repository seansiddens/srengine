#pragma once

#include "external/vk_mem_alloc.h"
#include "gpu_resources.h"
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
    void set_present_mode(PresentMode::Enum mode);

    // Swapchain
    bool create_swapchain();
    bool resize_swapchain();
    void destroy_swapchain();

    VkInstance vk_instance;
    VkDevice vk_device;
    VkPhysicalDevice vk_physical_device;
    VkPhysicalDeviceProperties vk_physical_device_properties;
    VkQueue vk_queue;
    u32 vk_queue_family;
    VkDescriptorPool vk_descriptor_pool;

    VkRenderPass vk_swapchain_renderpass;

    SDL_Window *window_handle;
    VkSurfaceKHR vk_surface;
    VkSurfaceFormatKHR vk_surface_format;
    VkPresentModeKHR vk_present_mode;
    VkSwapchainKHR vk_swapchain;
    u32 vk_swapchain_image_count;

    u32 swapchain_width = 1;
    u32 swapchain_height = 1;

    RenderPassOutput swapchain_output;

    PresentMode::Enum present_mode = PresentMode::VSync;
    u32 current_frame;
    u32 previous_frame;

    // Swapchain
    VkImage vk_swapchain_images[max_swapchain_images];
    VkImageView vk_swapchain_image_views[max_swapchain_images];
    VkFramebuffer vk_swapchain_framebuffers[max_swapchain_images];

    bool debug_utils_extension_present = false;
    VkDebugUtilsMessengerEXT vk_debug_utils_messenger;

    // NOTE: idk know what this is used for, but I have this handle in case I want to utilize
    // it later.
    VkAllocationCallbacks *vk_alloc_callbacks = nullptr;

    VmaAllocator vma_allocator;

    // Time (in seconds) required for a timestamp query's counter to increment by 1
    // TODO: I think this should be measured in ms instead.
    f32 gpu_timestamp_period;

    // NOTE: Idk what these are used for either.
    size_t ubo_alignment;
    size_t ssbo_alignment;
};

} // namespace sren