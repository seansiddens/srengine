#pragma once

#include <stdlib.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include "log.h"

// Macro for checking Vulkan callbacks
inline void vkAssert(VkResult result, bool abort = true) {
    if (result != VK_SUCCESS) {
        LOG_ERR("vkAssert: ", string_VkResult(result));
        exit(1);
    }
}

#define vkCheck(result)                                                                       \
    { vkAssert(result); }