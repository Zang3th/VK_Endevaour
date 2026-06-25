#pragma once

#include "Core/Types.hpp"

#include "Graphics/Vulkan/VulkanDevice.hpp"

// Forward-Declaration
using VmaAllocation = struct VmaAllocation_T*;

// Gets internally mapped to the corresponding vma enums
namespace Engine::Graphics
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

    struct BufferAllocation
    {
        vk::Buffer    Buffer;
        VmaAllocation Allocation;
    };

    struct BufferSpecification
    {
        vk::DeviceSize          Size;
        vk::BufferUsageFlags    BufferUsageFlags;
        MemoryUsage             MemoryUsage;
        vk::MemoryPropertyFlags MemoryFlags;
    };

    class VulkanAllocator
    {
    public:
        VulkanAllocator() = delete;

        static void Init(const VulkanDevice* device, vk::Instance instance, u32 apiVersion);
        static void Shutdown();

        static BufferAllocation AllocateBuffer(const BufferSpecification& spec);
        static void             DestroyBuffer(const BufferAllocation& bufferAlloc);

        static void* MapMemory(VmaAllocation allocation);
        static void  UnmapMemory(VmaAllocation allocation);
    };
}
