#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

namespace Engine::Graphics
{
    inline static const std::vector<const char*> g_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

    // clang-format off
    inline static const std::vector<const char*> g_DeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME
    };
    // clang-format on

    inline static constexpr u32 MAX_SHADER_COUNT   = 2;
    inline static constexpr u32 MAX_MODEL_COUNT    = 2;
    inline static constexpr u32 MAX_PIPELINE_COUNT = 1;

    inline static constexpr u32 FRAMES_IN_FLIGHT = 3;

    // Number of color attachments written by the graphics pipeline during dynamic rendering.
    // Currently only the swapchain color image at attachment location 0 is used.
    inline static constexpr u32 GLOBAL_COLOR_ATTACHMENT_COUNT = 1;

    // Number of descriptor set layouts used by the pipeline layout.
    // Currently only set 0, containing global frame uniforms, is defined.
    inline static constexpr u32 GLOBAL_DESCRIPTOR_SET_LAYOUT_COUNT = 1;

    // Number of uniform buffer descriptors at set 0, binding 0.
    // One descriptor references the complete GlobalUniformData buffer for the current frame.
    inline static constexpr u32 GLOBAL_UNIFORM_BINDING_DESCRIPTOR_COUNT = 1;
}
