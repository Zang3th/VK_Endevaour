#include "VulkanAllocator.hpp"
#include "VulkanAssert.hpp"

// Deactivate clang extensions
#define VMA_NULLABLE
#define VMA_NOT_NULL
#define VMA_NOT_NULL_NON_DISPATCHABLE
#include "Vendor/VulkanMemoryAllocator/vk_mem_alloc.hpp"

namespace
{
    // ----- Internal -----

    VmaAllocator s_Allocator   = VK_NULL_HANDLE;
    u64          s_totalMemory = 0;

    inline VmaMemoryUsage MapMemoryUsage(Engine::MemoryUsage usage)
    {
        switch(usage)
        {
            case Engine::MemoryUsage::eUnknown:          return VMA_MEMORY_USAGE_UNKNOWN;
            case Engine::MemoryUsage::eGPUOnly:          return VMA_MEMORY_USAGE_GPU_ONLY;
            case Engine::MemoryUsage::eCPUOnly:          return VMA_MEMORY_USAGE_CPU_ONLY;
            case Engine::MemoryUsage::eCPUToGPU:         return VMA_MEMORY_USAGE_CPU_TO_GPU;
            case Engine::MemoryUsage::eGPUToCPU:         return VMA_MEMORY_USAGE_GPU_TO_CPU;
            case Engine::MemoryUsage::eCPUCopy:          return VMA_MEMORY_USAGE_CPU_COPY;
            case Engine::MemoryUsage::eGPULazy:          return VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
            case Engine::MemoryUsage::eAuto:             return VMA_MEMORY_USAGE_AUTO;
            case Engine::MemoryUsage::eAutoPreferDevice: return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            case Engine::MemoryUsage::eAutoPreferHost:   return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        }
    };
}

namespace Engine
{
    // ----- Public -----

    void VulkanAllocator::Init(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance)
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice   = physicalDevice;
        allocatorInfo.device           = device;
        allocatorInfo.instance         = instance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

        VK_VERIFY((vk::Result)(vmaCreateAllocator(&allocatorInfo, &s_Allocator)));
        LOG_INFO("Created allocator with vma ...");
    }

    void VulkanAllocator::Shutdown()
    {
        vmaDestroyAllocator(s_Allocator);
    }

    VmaAllocation VulkanAllocator::AllocateBuffer(const vk::BufferCreateInfo& bufferCreateInfo, MemoryUsage usage, vk::Buffer& outBuffer)
    {
        ASSERT(bufferCreateInfo.size > 0, "Provided buffer size was less or equal to zero!");

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = MapMemoryUsage(usage);

        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo{};
        VK_VERIFY((vk::Result)(vmaCreateBuffer(
            s_Allocator,
            (const VkBufferCreateInfo*)&bufferCreateInfo,
            &allocCreateInfo,
            (VkBuffer*)&outBuffer,
            &allocation, &allocationInfo
        )));
        s_totalMemory += allocationInfo.size;

        LOG_VERBOSE("Allocator allocated {} bytes of memory. Total memory consumption is {} bytes ...", allocationInfo.size, s_totalMemory);

        return allocation;
    }

    void VulkanAllocator::DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(s_Allocator, (VkBuffer)buffer, allocation);
    }
}
