#pragma once

#include "Core/Types.hpp"

#include "vulkan/vulkan.hpp"

// Forward-Declaration
using VmaAllocation = struct VmaAllocation_T*;

// Gets internally mapped to the corresponding vma enums
namespace Engine
{
    enum class MemoryUsage : u8
    {
        eUnknown          = 0,
        eGPUOnly          = 1,
        eCPUOnly          = 2,
        eCPUToGPU         = 3,
        eGPUToCPU         = 4,
        eCPUCopy          = 5,
        eGPULazy          = 6,
        eAuto             = 7,
        eAutoPreferDevice = 8,
        eAutoPreferHost   = 9
    };

    class VulkanAllocator
    {
        public:
            VulkanAllocator() = delete;

            static void Init(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance);
            static void Shutdown();
            static VmaAllocation AllocateBuffer(const vk::BufferCreateInfo& bufferCreateInfo, MemoryUsage usage, vk::Buffer& outBuffer);
            static void DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation);
    };
}
