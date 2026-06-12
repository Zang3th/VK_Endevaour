#include "VulkanAllocator.hpp"

#include "Core/Utility.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"

// Deactivate clang extensions
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wunused-private-field"

#define VMA_IMPLEMENTATION
#define VMA_NULLABLE
#define VMA_NOT_NULL
#define VMA_NOT_NULL_NON_DISPATCHABLE
#include "Vendor/VulkanMemoryAllocator/vk_mem_alloc.hpp"

#pragma clang diagnostic pop

namespace
{
    // ----- Internal -----

    VmaAllocator s_Allocator   = VK_NULL_HANDLE;
    Engine::u64  s_totalMemory = 0;

    inline VmaMemoryUsage MapMemoryUsage(Engine::Graphics::MemoryUsage usage)
    {
        switch (usage)
        {
        case Engine::Graphics::MemoryUsage::eUnknown:
            return VMA_MEMORY_USAGE_UNKNOWN;
        case Engine::Graphics::MemoryUsage::eGPUOnly:
            return VMA_MEMORY_USAGE_GPU_ONLY;
        case Engine::Graphics::MemoryUsage::eCPUOnly:
            return VMA_MEMORY_USAGE_CPU_ONLY;
        case Engine::Graphics::MemoryUsage::eCPUToGPU:
            return VMA_MEMORY_USAGE_CPU_TO_GPU;
        case Engine::Graphics::MemoryUsage::eGPUToCPU:
            return VMA_MEMORY_USAGE_GPU_TO_CPU;
        case Engine::Graphics::MemoryUsage::eCPUCopy:
            return VMA_MEMORY_USAGE_CPU_COPY;
        case Engine::Graphics::MemoryUsage::eGPULazy:
            return VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
        case Engine::Graphics::MemoryUsage::eAuto:
            return VMA_MEMORY_USAGE_AUTO;
        case Engine::Graphics::MemoryUsage::eAutoPreferDevice:
            return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        case Engine::Graphics::MemoryUsage::eAutoPreferHost:
            return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        }
    };
}

namespace Engine::Graphics
{
    // ----- Public -----

    void VulkanAllocator::Init(const VulkanDevice* device, vk::Instance instance, u32 apiVersion)
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice   = device->GetPhysicalDevice()->GetHandle();
        allocatorInfo.device           = device->GetHandle();
        allocatorInfo.instance         = instance;
        allocatorInfo.vulkanApiVersion = apiVersion;

        VK_VERIFY((vk::Result)(vmaCreateAllocator(&allocatorInfo, &s_Allocator)));
        LOG_INFO("Created allocator with vma ...");
    }

    std::pair<vk::Buffer, VmaAllocation> VulkanAllocator::AllocateBuffer(vk::DeviceSize       size,
                                                                         vk::BufferUsageFlags usage,
                                                                         MemoryUsage          memoryUsage)
    {
        const vk::BufferCreateInfo bufferInfo{ .size        = size,
                                               .usage       = usage,
                                               .sharingMode = vk::SharingMode::eExclusive };

        vk::Buffer    buffer;
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

        LOG_MEMORY("Allocator freed {} of memory. Total memory consumption is {} ...",
                   Core::Utility::BytesToString(allocationInfo.size),
                   Core::Utility::BytesToString(s_totalMemory));
    }

    void* VulkanAllocator::MapMemory(VmaAllocation allocation)
    {
        void* dataPtr = nullptr;
        VK_VERIFY((vk::Result)vmaMapMemory(s_Allocator, allocation, &dataPtr));
        return dataPtr;
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(s_Allocator, allocation);
    }

    // ----- Private -----

    VmaAllocation VulkanAllocator::InternalAllocateBuffer(const vk::BufferCreateInfo& bufferCreateInfo,
                                                          MemoryUsage                 usage,
                                                          vk::Buffer&                 outBuffer)
    {
        ASSERT(bufferCreateInfo.size > 0, "Provided buffer size was less or equal to zero!");

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage                   = MapMemoryUsage(usage);

        VmaAllocation     allocation;
        VmaAllocationInfo allocationInfo{};
        VK_VERIFY((vk::Result)(vmaCreateBuffer(s_Allocator,
                                               (const VkBufferCreateInfo*)&bufferCreateInfo,
                                               &allocCreateInfo,
                                               (VkBuffer*)&outBuffer,
                                               &allocation,
                                               &allocationInfo)));
        s_totalMemory += allocationInfo.size;

        LOG_MEMORY("Allocator allocated {} of memory. Total memory consumption is {} ...",
                   Core::Utility::BytesToString(allocationInfo.size),
                   Core::Utility::BytesToString(s_totalMemory));

        return allocation;
    }
}
