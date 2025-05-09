#include "VulkanAllocator.hpp"
#include "VulkanAssert.hpp"

#include "Core/Utility.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wunused-private-field"

#define VMA_IMPLEMENTATION

// Deactivate clang extensions
#define VMA_NULLABLE
#define VMA_NOT_NULL
#define VMA_NOT_NULL_NON_DISPATCHABLE
#include "Vendor/VulkanMemoryAllocator/vk_mem_alloc.hpp"

#pragma clang diagnostic pop

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

    std::pair<vk::Buffer, VmaAllocation> VulkanAllocator::AllocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, MemoryUsage memoryUsage)
    {
        vk::BufferCreateInfo bufferInfo
        {
            .size        = size,
            .usage       = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        vk::Buffer buffer;
        VmaAllocation allocation = VulkanAllocator::InternalAllocateBuffer(bufferInfo, memoryUsage, buffer);

        return { buffer, allocation };
    }

    void VulkanAllocator::Shutdown()
    {
        vmaDestroyAllocator(s_Allocator);
    }

    void VulkanAllocator::DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation)
    {
        VmaAllocationInfo allocationInfo{};
        vmaGetAllocationInfo(s_Allocator, allocation, &allocationInfo);
        s_totalMemory -= allocationInfo.size;

        vmaDestroyBuffer(s_Allocator, (VkBuffer)buffer, allocation);

        LOG_VERBOSE("Allocator freed {} of memory. Total memory consumption is {} ...", Utility::BytesToString(allocationInfo.size), Utility::BytesToString(s_totalMemory));
    }

    void* VulkanAllocator::MapMemory(VmaAllocation allocation)
    {
        void* dataPtr;
        vmaMapMemory(s_Allocator, allocation, &dataPtr);
        return dataPtr;
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(s_Allocator, allocation);
    }

    // ----- Private -----

    VmaAllocation VulkanAllocator::InternalAllocateBuffer(const vk::BufferCreateInfo& bufferCreateInfo, MemoryUsage usage, vk::Buffer& outBuffer)
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

        LOG_VERBOSE("Allocator allocated {} of memory. Total memory consumption is {} ...", Utility::BytesToString(allocationInfo.size), Utility::BytesToString(s_totalMemory));

        return allocation;
    }
}
