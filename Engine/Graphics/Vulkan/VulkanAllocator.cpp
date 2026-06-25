#include "VulkanAllocator.hpp"

#include "Core/Utility.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"

#include <string_view>

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

    inline std::string_view MemoryUsageToString(Engine::Graphics::MemoryUsage usage)
    {
        switch (usage)
        {
        case Engine::Graphics::MemoryUsage::eUnknown:
            return "Unknown";
        case Engine::Graphics::MemoryUsage::eGPUOnly:
            return "GPUOnly";
        case Engine::Graphics::MemoryUsage::eCPUOnly:
            return "CPUOnly";
        case Engine::Graphics::MemoryUsage::eCPUToGPU:
            return "CPUToGPU";
        case Engine::Graphics::MemoryUsage::eGPUToCPU:
            return "GPUToCPU";
        case Engine::Graphics::MemoryUsage::eCPUCopy:
            return "CPUCopy";
        case Engine::Graphics::MemoryUsage::eGPULazy:
            return "GPULazy";
        case Engine::Graphics::MemoryUsage::eAuto:
            return "Auto";
        case Engine::Graphics::MemoryUsage::eAutoPreferDevice:
            return "AutoPreferDevice";
        case Engine::Graphics::MemoryUsage::eAutoPreferHost:
            return "AutoPreferHost";
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

    BufferAllocation VulkanAllocator::AllocateBuffer(const BufferSpecification& spec)
    {
        ASSERT(spec.Size > 0, "Provided buffer size was less or equal to zero!");

        const vk::BufferCreateInfo bufferInfo{ .size        = spec.Size,
                                               .usage       = spec.BufferUsageFlags,
                                               .sharingMode = vk::SharingMode::eExclusive };

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.requiredFlags = (VkMemoryPropertyFlags)spec.MemoryFlags;
        allocCreateInfo.usage         = MapMemoryUsage(spec.MemoryUsage);

        vk::Buffer        buffer;
        VmaAllocation     allocation{};
        VmaAllocationInfo allocationInfo{};

        VK_VERIFY((vk::Result)(vmaCreateBuffer(s_Allocator,
                                               (const VkBufferCreateInfo*)&bufferInfo,
                                               &allocCreateInfo,
                                               (VkBuffer*)&buffer,
                                               &allocation,
                                               &allocationInfo)));
        s_totalMemory += allocationInfo.size;

        LOG_PERF("Allocated {} of '{}' memory as {}. Total: {} ...",
                 Core::Utility::BytesToString(allocationInfo.size),
                 MemoryUsageToString(spec.MemoryUsage),
                 vk::to_string(spec.BufferUsageFlags),
                 Core::Utility::BytesToString(s_totalMemory));

        return { .Buffer = buffer, .Allocation = allocation };
    }

    void VulkanAllocator::Shutdown()
    {
        vmaDestroyAllocator(s_Allocator);
    }

    void VulkanAllocator::DestroyBuffer(const BufferAllocation& bufferAlloc)
    {
        VmaAllocationInfo allocationInfo{};
        vmaGetAllocationInfo(s_Allocator, bufferAlloc.Allocation, &allocationInfo);

        VkMemoryPropertyFlags memoryFlags{};
        vmaGetAllocationMemoryProperties(s_Allocator, bufferAlloc.Allocation, &memoryFlags);

        s_totalMemory -= allocationInfo.size;
        vmaDestroyBuffer(s_Allocator, (VkBuffer)bufferAlloc.Buffer, bufferAlloc.Allocation);

        LOG_PERF("Freed {} from {} memory. Total: {} ...",
                 Core::Utility::BytesToString(allocationInfo.size),
                 vk::to_string((vk::MemoryPropertyFlags)memoryFlags),
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
}
