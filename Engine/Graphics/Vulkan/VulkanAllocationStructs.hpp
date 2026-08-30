#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

#include <string_view>

namespace Engine::Graphics
{
    enum class MemoryUsage : u8
    {
        eUnknown          = 0u,
        eGPUOnly          = 1u,
        eCPUOnly          = 2u,
        eCPUToGPU         = 3u,
        eGPUToCPU         = 4u,
        eCPUCopy          = 5u,
        eGPULazy          = 6u,
        eAuto             = 7u,
        eAutoPreferDevice = 8u,
        eAutoPreferHost   = 9u
    };
    struct BufferAllocation
    {
        vk::Buffer       Buffer = nullptr;
        void*            Handle = nullptr;
        std::string_view DebugName;
    };

    struct BufferSpecification
    {
        vk::DeviceSize          Size;
        vk::BufferUsageFlags    BufferUsageFlags;
        MemoryUsage             MemoryUsage;
        vk::MemoryPropertyFlags MemoryFlags;
        std::string_view        DebugName;
    };

    struct ImageAllocation
    {
        void*            Handle = nullptr;
        std::string_view DebugName;
    };

    struct ImageSpecification
    {
        const vk::ImageCreateInfo* CreateInfo = nullptr;
        vk::Image*                 Image      = nullptr;
        MemoryUsage                MemoryUsage;
        std::string_view           DebugName;
    };
}
