#pragma once

#include "Core/Types.hpp"

#include "Graphics/Vulkan/VulkanDevice.hpp"

// Gets internally mapped to the corresponding vma enums
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
        vk::Buffer Buffer = nullptr;
        void*      Handle = nullptr;
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

        [[nodiscard]] static BufferAllocation AllocateBuffer(const BufferSpecification& spec);
        static void                           DestroyBuffer(const BufferAllocation& bufferAlloc);

        [[nodiscard]] static void* AllocateImage(const vk::ImageCreateInfo* imageCreateInfo, vk::Image* image);
        static void                DestroyImage(vk::Image image, void* allocationHandle);

        [[nodiscard]] static void* MapMemory(void* allocationHandle);
        static void                UnmapMemory(void* allocationHandle);
    };
}
