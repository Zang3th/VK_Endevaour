#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanDescriptorPool.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanDescriptorPool::VulkanDescriptorPool(vk::Device device, const DescriptorPoolSpecification& spec)
        : m_Device(device)
    {
        Create(spec);
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        LOG_INFO("VulkanDescriptorPool::Destructor() ...");
        m_Device.destroyDescriptorPool(m_Pool);
    }

    // ----- Private -----

    void VulkanDescriptorPool::Create(const DescriptorPoolSpecification& spec)
    {
        const vk::DescriptorPoolCreateInfo poolCreateInfo{ .flags         = spec.Flags,
                                                           .maxSets       = spec.MaxSets,
                                                           .poolSizeCount = (u32)(spec.PoolSizes.size()),
                                                           .pPoolSizes    = spec.PoolSizes.data() };

        VK_VERIFY(m_Device.createDescriptorPool(&poolCreateInfo, nullptr, &m_Pool));
        LOG_INFO("Created descriptor pool ... (MaxSets: {}, PoolSize: {})", spec.MaxSets, spec.PoolSizes.size());
    }
}
