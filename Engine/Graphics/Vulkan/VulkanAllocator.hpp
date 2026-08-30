#pragma once

#include "Graphics/Vulkan/VulkanAllocationStructs.hpp"

namespace Engine::Graphics
{
    class VulkanDevice;
}

namespace Engine::Graphics
{
    class VulkanAllocator
    {
    public:
        VulkanAllocator() = delete;

        static void Init(const VulkanDevice* device, vk::Instance instance, u32 apiVersion);
        static void Shutdown();

        [[nodiscard]] static BufferAllocation AllocateBuffer(const BufferSpecification& spec);
        static void                           DestroyBuffer(const BufferAllocation& alloc);

        [[nodiscard]] static ImageAllocation AllocateImage(const ImageSpecification& spec);
        static void                          DestroyImage(vk::Image image, const ImageAllocation& alloc);

        [[nodiscard]] static void* MapMemory(void* allocationHandle);
        static void                UnmapMemory(void* allocationHandle);
    };
}
