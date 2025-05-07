#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

#ifdef NDEBUG
    const bool ENABLE_VALIDATION_LAYERS = false;
#else
    const bool ENABLE_VALIDATION_LAYERS = true;
#endif

namespace Engine
{
    inline static const std::vector<const char*> g_ValidationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    inline static const std::vector<const char*> g_DeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}
