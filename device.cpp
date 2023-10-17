#include "device.h"

#include "vk_common.h"

#include <SDL2/SDL_vulkan.h>
#include <string.h>
#include <vector>

namespace sren {

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

bool Device::init(u32 window_width, u32 window_height, SDL_Window *window) {
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

    swapchain_width = window_width;
    swapchain_height = window_height;

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
    LOG_DBG("Selected physical device: %s", vk_physical_device_properties.deviceName);

    LOG_DBG("Initialized device.");
    return true;
}

void Device::teardown() {
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
} // namespace sren