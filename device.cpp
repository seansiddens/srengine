#include "device.h"

#include "vk_common.h"

#include <SDL2/SDL_vulkan.h>
#include <cassert>
#include <string.h>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wparentheses"
#define VMA_IMPLEMENTATION
#include "external/vk_mem_alloc.h"
#pragma GCC diagnostic pop

namespace sren {

PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT;

template <class T> constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
    assert(!(hi < lo));
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

// Validation layers to enable.
static const char *requested_layers[] = {
#ifdef VULKAN_DEBUG_REPORT
    "VK_LAYER_KHRONOS_validation",
//"VK_LAYER_LUNARG_core_validation",
//"VK_LAYER_LUNARG_image",
//"VK_LAYER_LUNARG_parameter_validation",
//"VK_LAYER_LUNARG_object_tracker"
#else
    "",
#endif
};

// Extensions to request.
static std::vector<const char *> requested_extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VULKAN_DEBUG_REPORT
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

#ifdef VULKAN_DEBUG_REPORT
static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                     VkDebugUtilsMessageTypeFlagsEXT,
                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *) {
    LOG_ERR("MessageID: %s %i\nMessage: %s\n", callback_data->pMessageIdName,
            callback_data->messageIdNumber, callback_data->pMessage);

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        // __debugbreak();
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT create_debug_utils_messenger_info() {
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.pfnUserCallback = debug_utils_callback;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    return create_info;
}
#endif // VULKAN_DEBUG_REPORT

bool Device::get_family_queue(VkPhysicalDevice physical_device) {
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    VkQueueFamilyProperties *queue_families =
        (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

    u32 family_index = 0;
    VkBool32 surface_supported;
    for (; family_index < queue_family_count; ++family_index) {
        VkQueueFamilyProperties queue_family = queue_families[family_index];
        if (queue_family.queueCount > 0 &&
            queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, vk_surface,
                                                 &surface_supported);
            if (surface_supported) {
                vk_queue_family = family_index;
                break;
            }
        }
    }
    free(queue_families);
    return surface_supported;
}

bool Device::init(u32 window_width_, u32 window_height_, SDL_Window *window) {
    // 1. Initialize Vulkan instance.
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "Srengine App";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Sren Engine";
    app_info.apiVersion = VK_API_VERSION_1_3;

    // Query SDL required extensions.
    // TODO: Is there a more platform/window agnostic way of doing this?
    u32 sdl_extension_count;
    if (SDL_Vulkan_GetInstanceExtensions(window, &sdl_extension_count, nullptr) == SDL_FALSE) {
        LOG_ERR("Failed to enumerate SDL extensions: %s", SDL_GetError());
        return false;
    }
    const char **sdl_extensions = (const char **)malloc(sizeof(const char *) * sdl_extension_count);
    if (SDL_Vulkan_GetInstanceExtensions(window, &sdl_extension_count, sdl_extensions) == SDL_FALSE) {
        LOG_ERR("Failed to get SDL instance extensions: %s", SDL_GetError());
        free(sdl_extensions);
        return false;
    }
    // Add these extensions to our requested extensions.
    for (u32 i = 0; i < sdl_extension_count; i++) {
        requested_extensions.push_back(sdl_extensions[i]);
    }
    free(sdl_extensions);

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.flags = 0;
    create_info.pApplicationInfo = &app_info;
#ifdef VULKAN_DEBUG_REPORT
    // Set debug layers, if applicable.
    const VkDebugUtilsMessengerCreateInfoEXT debug_create_info = create_debug_utils_messenger_info();
    create_info.pNext = &debug_create_info;
    create_info.enabledLayerCount = sizeof(requested_layers) / sizeof(requested_layers[0]);
    create_info.ppEnabledLayerNames = requested_layers;
#else
    create_info.pNext = nullptr;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
#endif
    create_info.enabledExtensionCount = requested_extensions.size();
    create_info.ppEnabledExtensionNames = requested_extensions.data();

    // Create Vulkan instance.
    if (!vkCheck(vkCreateInstance(&create_info, vk_alloc_callbacks, &vk_instance))) {
        return false;
    }
    LOG_DBG("Created Vulkan instance.");

    swapchain_width = window_width_;
    swapchain_height = window_height_;

// Choose extensions.
#ifdef VULKAN_DEBUG_REPORT
    {
        u32 num_instance_extensions;
        vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
        VkExtensionProperties *extensions =
            (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * num_instance_extensions);
        vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, extensions);
        for (size_t i = 0; i < num_instance_extensions; i++) {
            if (!strcmp(extensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
                debug_utils_extension_present = true;
                break;
            }
        }
        free(extensions);

        if (!debug_utils_extension_present) {
            LOG_DBG("Extension %s for debugging non present.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        } else {
            // Create new debug utils callback
            PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
                (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                    vk_instance, "vkCreateDebugUtilsMessengerEXT");
            VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info =
                create_debug_utils_messenger_info();

            vkCreateDebugUtilsMessengerEXT(vk_instance, &debug_messenger_create_info, vk_alloc_callbacks,
                                           &vk_debug_utils_messenger);
        }
    }
#endif
    // TODO: Do I have to do anything for the other extensions I request?

    // Choose physical device.
    // TODO: Do I want a way to manually select a device?
    u32 num_physical_device;
    if (!vkCheck(vkEnumeratePhysicalDevices(vk_instance, &num_physical_device, nullptr))) {
        return false;
    }
    VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * num_physical_device);
    if (!vkCheck(vkEnumeratePhysicalDevices(vk_instance, &num_physical_device, gpus))) {
        return false;
    }

    // Create drawable surface.
    window_handle = window;
    if (SDL_Vulkan_CreateSurface(window_handle, vk_instance, &vk_surface) == SDL_FALSE) {
        LOG_ERR("Failed to create window surface: %s", SDL_GetError());
        return false;
    }

    VkPhysicalDevice discrete_gpu = VK_NULL_HANDLE;
    VkPhysicalDevice integrated_gpu = VK_NULL_HANDLE;
    // TODO: More sophisticated device selection, e.g if there are multiple gpus w/ the required
    // capabilities, give ability to select.
    for (u32 i = 0; i < num_physical_device; ++i) {
        VkPhysicalDevice physical_device = gpus[i];
        vkGetPhysicalDeviceProperties(physical_device, &vk_physical_device_properties);
        if (vk_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            if (get_family_queue(physical_device)) {
                // NOTE: prefer discrete GPU over integrated one, stop at first discrete GPU that
                // has present capabilities
                discrete_gpu = physical_device;
                break;
            }
            continue;
        }
        if (vk_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            if (get_family_queue(physical_device)) {
                integrated_gpu = physical_device;
            }
            continue;
        }
    }

    if (discrete_gpu != VK_NULL_HANDLE) {
        vk_physical_device = discrete_gpu;
    } else if (integrated_gpu != VK_NULL_HANDLE) {
        vk_physical_device = integrated_gpu;
    } else {
        LOG_ERR("Suitable GPU device not found!");
        return false;
    }
    free(gpus);

    // Query physical device properties.
    vkGetPhysicalDeviceProperties(vk_physical_device, &vk_physical_device_properties);
    LOG_DBG("Selected GPU: %s", vk_physical_device_properties.deviceName);
    gpu_timestamp_period = vk_physical_device_properties.limits.timestampPeriod / (1000 * 1000);
    LOG_DBG("GPU timestamp period: %f", gpu_timestamp_period);
    ubo_alignment = vk_physical_device_properties.limits.minUniformBufferOffsetAlignment;
    ssbo_alignment = vk_physical_device_properties.limits.minStorageBufferOffsetAlignment;

    // 2. Create logical device.
    u32 device_extension_count = 1;
    const char *device_extensions[] = {"VK_KHR_swapchain"};
    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = vk_queue_family;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;

    // Enable all features: just pass the physical features 2 struct.
    VkPhysicalDeviceFeatures2 physical_features2;
    physical_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physical_features2.pNext = nullptr;
    physical_features2.features = {};
    vkGetPhysicalDeviceFeatures2(vk_physical_device, &physical_features2);

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    device_create_info.pQueueCreateInfos = queue_info;
    device_create_info.enabledExtensionCount = device_extension_count;
    device_create_info.ppEnabledExtensionNames = device_extensions;
    device_create_info.pNext = &physical_features2;
    if (!vkCheck(
            vkCreateDevice(vk_physical_device, &device_create_info, vk_alloc_callbacks, &vk_device))) {
        return false;
    };
    LOG_DBG("Created logical device.");

    //  Get the function pointers to Debug Utils functions.
    if (debug_utils_extension_present) {
        pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(
            vk_device, "vkSetDebugUtilsObjectNameEXT");
        pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(
            vk_device, "vkCmdBeginDebugUtilsLabelEXT");
        pfnCmdEndDebugUtilsLabelEXT =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(vk_device, "vkCmdEndDebugUtilsLabelEXT");
    }

    vkGetDeviceQueue(vk_device, vk_queue_family, 0, &vk_queue);

    // 3. Create framebuffers.
    int window_width, window_height;
    SDL_GetWindowSize(window_handle, &window_width, &window_height);

    // Select surface format.
    const VkFormat surface_image_formats[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                              VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    u32 supported_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &supported_count, NULL);
    VkSurfaceFormatKHR *supported_formats =
        (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * supported_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &supported_count,
                                         supported_formats);

    swapchain_output.reset();

    // Check for supported formats.
    bool format_found = false;
    const u32 surface_format_count = sizeof(surface_image_formats) / sizeof(surface_image_formats[0]);

    for (u32 i = 0; i < surface_format_count; i++) {
        for (u32 j = 0; j < supported_count; j++) {
            if (supported_formats[j].format == surface_image_formats[i] &&
                supported_formats[j].colorSpace == surface_color_space) {
                vk_surface_format = supported_formats[j];
                swapchain_output.color(surface_image_formats[j]);
                format_found = true;
                break;
            }
        }

        if (format_found)
            break;
    }

    if (!format_found) {
        // Default to the first format supported.
        vk_surface_format = supported_formats[0];
        LOG_ERR("Failed to find supported surface format");
    }
    free(supported_formats);

    set_present_mode(present_mode);

    // Create swapchain
    if (!create_swapchain()) {
        LOG_ERR("Failed to create swapchain!");
        return false;
    }

    // Create VMA allocator.
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = vk_physical_device;
    allocator_info.device = vk_device;
    allocator_info.instance = vk_instance;
    if (!vkCheck(vmaCreateAllocator(&allocator_info, &vma_allocator))) {
        LOG_ERR("Failed to create VMA allocator.")
        return false;
    }
    ////////  Create pools
    static const u32 global_pool_elements = 128;
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, global_pool_elements},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, global_pool_elements}};
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = global_pool_elements * (sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.poolSizeCount = (u32)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.pPoolSizes = pool_sizes;
    if (!vkCheck(
            vkCreateDescriptorPool(vk_device, &pool_info, vk_alloc_callbacks, &vk_descriptor_pool))) {
        LOG_ERR("Failed to create descriptor pool.");
        return false;
    }

    LOG_DBG("Initialized Device.");
    return true;
}

void Device::teardown() {
    destroy_swapchain();

    vkDestroyDevice(vk_device, vk_alloc_callbacks);

    vkDestroySurfaceKHR(vk_instance, vk_surface, vk_alloc_callbacks);

#ifdef VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        vk_instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_utils_messenger, vk_alloc_callbacks);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyInstance(vk_instance, vk_alloc_callbacks);

    LOG_DBG("Device cleaned up.");
}

bool Device::create_swapchain() {
    //// Check if surface is supported
    VkBool32 surface_supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, vk_queue_family, vk_surface,
                                         &surface_supported);
    if (surface_supported != VK_TRUE) {
        LOG_ERR("Error no WSI support on physical device 0\n");
        return false;
    }

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surface_capabilities);

    VkExtent2D swapchain_extent = surface_capabilities.currentExtent;
    if (swapchain_extent.width == UINT32_MAX) {
        swapchain_extent.width = clamp(swapchain_extent.width, surface_capabilities.minImageExtent.width,
                                       surface_capabilities.maxImageExtent.width);
        swapchain_extent.height =
            clamp(swapchain_extent.height, surface_capabilities.minImageExtent.height,
                  surface_capabilities.maxImageExtent.height);
    }

    LOG_DBG("Create swapchain %u %u - saved %u %u, min image %u\n", swapchain_extent.width,
            swapchain_extent.height, swapchain_width, swapchain_height,
            surface_capabilities.minImageCount);

    swapchain_width = swapchain_extent.width;
    swapchain_height = swapchain_extent.height;

    // vulkan_swapchain_image_count = surface_capabilities.minImageCount + 2;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = vk_surface;
    swapchain_create_info.minImageCount = vk_swapchain_image_count;
    swapchain_create_info.imageFormat = vk_surface_format.format;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = vk_present_mode;

    if (!vkCheck(vkCreateSwapchainKHR(vk_device, &swapchain_create_info, 0, &vk_swapchain))) {
        return false;
    }

    // Cache swapchain images
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &vk_swapchain_image_count, nullptr);
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &vk_swapchain_image_count, vk_swapchain_images);

    for (size_t iv = 0; iv < vk_swapchain_image_count; iv++) {
        // Create an image view which we can render into.
        VkImageViewCreateInfo view_info;
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.pNext = nullptr;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = vk_surface_format.format;
        view_info.image = vk_swapchain_images[iv];
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.layerCount = 1;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.components.r = VK_COMPONENT_SWIZZLE_R;
        view_info.components.g = VK_COMPONENT_SWIZZLE_G;
        view_info.components.b = VK_COMPONENT_SWIZZLE_B;
        view_info.components.a = VK_COMPONENT_SWIZZLE_A;
        view_info.flags = 0;

        if (!vkCheck(vkCreateImageView(vk_device, &view_info, vk_alloc_callbacks,
                                       &vk_swapchain_image_views[iv]))) {
            return false;
        }
    }

    return true;
}

bool Device::resize_swapchain() {
    vkDeviceWaitIdle(vk_device);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surface_capabilities);
    VkExtent2D swapchain_extent = surface_capabilities.currentExtent;

    // Skip zero-sized swapchain
    // rprint( "Requested swapchain resize %u %u\n", swapchain_extent.width, swapchain_extent.height );
    if (swapchain_extent.width == 0 || swapchain_extent.height == 0) {
        LOG_ERR("Cannot create a zero-sized swapchain");
        return false;
    }

    // Internal destroy of swapchain pass to retain the same handle.
    // TODO: Why do we do this?
    vkDestroyRenderPass(vk_device, vk_swapchain_renderpass, vk_alloc_callbacks);

    // Destroy swapchain images and framebuffers
    destroy_swapchain();
    vkDestroySurfaceKHR(vk_instance, vk_surface, vk_alloc_callbacks);

    // Recreate window surface
    if (SDL_Vulkan_CreateSurface(window_handle, vk_instance, &vk_surface) == SDL_FALSE) {
        LOG_ERR("Failed to re-create Vulkan surface.");
    }

    // Create swapchain
    create_swapchain();

    // Resize depth texture, maintaining handle, using a dummy texture to destroy.
    // TODO: Implement me.
    // TextureHandle texture_to_delete = {textures.obtain_resource()};
    // Texture *vk_texture_to_delete = access_texture(texture_to_delete);
    // vk_texture_to_delete->handle = texture_to_delete;
    // Texture *vk_depth_texture = access_texture(depth_texture);
    // vulkan_resize_texture(*this, vk_depth_texture, vk_texture_to_delete, swapchain_width,
    //                       swapchain_height, 1);

    // destroy_texture(texture_to_delete);

    // RenderPassCreation swapchain_pass_creation = {};
    // swapchain_pass_creation.set_type(RenderPassType::Swapchain).set_name("Swapchain");
    // vulkan_create_swapchain_pass(*this, swapchain_pass_creation, vk_swapchain_pass);

    vkDeviceWaitIdle(vk_device);
    return true;
}

void Device::destroy_swapchain() {
    for (size_t iv = 0; iv < vk_swapchain_image_count; iv++) {
        vkDestroyImageView(vk_device, vk_swapchain_image_views[iv], vk_alloc_callbacks);
        vkDestroyFramebuffer(vk_device, vk_swapchain_framebuffers[iv], vk_alloc_callbacks);
    }

    vkDestroySwapchainKHR(vk_device, vk_swapchain, vk_alloc_callbacks);
}

static VkPresentModeKHR to_vk_present_mode(PresentMode::Enum mode) {
    switch (mode) {
    case PresentMode::VSyncFast:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    case PresentMode::VSyncRelaxed:
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    case PresentMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case PresentMode::VSync:
    default:
        return VK_PRESENT_MODE_FIFO_KHR;
    }
}

void Device::set_present_mode(PresentMode::Enum mode) {
    // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR which
    // is mandatory
    u32 supported_count = 0;

    static VkPresentModeKHR supported_mode_allocated[8];
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &supported_count, NULL);
    assert(supported_count < 8);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &supported_count,
                                              supported_mode_allocated);

    bool mode_found = false;
    VkPresentModeKHR requested_mode = to_vk_present_mode(mode);
    for (u32 j = 0; j < supported_count; j++) {
        if (requested_mode == supported_mode_allocated[j]) {
            mode_found = true;
            break;
        }
    }

    // TODO: This chunk of code does not look right.
    // Default to VK_PRESENT_MODE_FIFO_KHR that is guaranteed to always be supported
    vk_present_mode = mode_found ? requested_mode : VK_PRESENT_MODE_FIFO_KHR;
    // Use 4 for immediate ?
    vk_swapchain_image_count = 3; // vulkan_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR ? 2 : 3;

    present_mode = mode_found ? mode : PresentMode::VSync;
}

} // namespace sren