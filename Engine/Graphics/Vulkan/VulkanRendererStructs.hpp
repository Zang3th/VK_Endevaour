#pragma once

#include "Graphics/Vulkan/VulkanSwapchainStructs.hpp"

#include <optional>

namespace Engine::Graphics
{
    struct RenderPacket
    {
        std::optional<SwapchainFrame> Frame;
        u32                           PipelineID = 0;

        [[nodiscard]] bool IsValid() const { return Frame.has_value(); }
    };

    struct RenderStats
    {
        u32 DrawCalls = 0;
        u32 Models    = 0;
        u32 Vertices  = 0;
        u32 Indices   = 0;
    };
}
