#include "device.h"

#include "vk_common.h"

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
static const char *requested_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VULKAN_DEBUG_REPORT
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

#ifdef VULKAN_DEBUG_REPORT
static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                     VkDebugUtilsMessageTypeFlagsEXT,
                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                     void *) {
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
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    return create_info;
}
#endif // VULKAN_DEBUG_REPORT

bool Device::init(u32 window_width, u32 window_height) {
    // 1. Initialize Vulkan instance.
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "Srengine App";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Sren Engine";
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.flags = 0;
    create_info.pApplicationInfo = &app_info;
#ifdef VULKAN_DEBUG_REPORT
    // Set debug layers, if applicable.
    const VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
        create_debug_utils_messenger_info();
    create_info.pNext = &debug_create_info;
    create_info.enabledLayerCount = sizeof(requested_layers) / sizeof(requested_layers[0]);
    create_info.ppEnabledLayerNames = requested_layers;
#else
    create_info.pNext = nullptr;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
#endif
    create_info.enabledExtensionCount =
        sizeof(requested_extensions) / sizeof(requested_extensions[0]);
    create_info.ppEnabledExtensionNames = requested_extensions;

    // Create Vulkan instance.
    vkCheck(vkCreateInstance(&create_info, nullptr, &instance));
    LOG_DBG("Created Vulkan instance.");

    swapchain_width = window_width;
    swapchain_height = window_height;

    LOG_DBG("Initialized device.");
    return true;
}

void Device::teardown() {
    vkDestroyInstance(instance, nullptr);

    LOG_DBG("Device cleaned up.");
}
} // namespace sren