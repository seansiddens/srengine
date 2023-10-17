#pragma once

#include <stdlib.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include "log.h"

// Function to check Vulkan callbacks and return a boolean value
inline bool vkAssert(VkResult result, bool abort = true) {
    if (result != VK_SUCCESS) {
        LOG_ERR("vkAssert: ", string_VkResult(result));
        if (abort) {
            exit(1);
        }
        return false;
    }
    return true;
}

// Modified vkCheck macro to return true or false
#define vkCheck(result) (vkAssert(result))

// Enable this to add debugging capabilities.
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_debug_utils.html
#define VULKAN_DEBUG_REPORT