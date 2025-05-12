#pragma once

#include "Core/Types.hpp"

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
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };

    inline static constexpr u32 MAX_SHADER_COUNT   = 2;
    inline static constexpr u32 MAX_MODEL_COUNT    = 1;
    inline static constexpr u32 MAX_PIPELINE_COUNT = 1;
}
