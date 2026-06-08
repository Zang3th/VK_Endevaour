#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanDescriptorPool.hpp"

namespace Engine::Graphics
{
    VulkanDescriptorPool::VulkanDescriptorPool(vk::Device device, const DescriptorPoolSpecification& spec)
        : m_Device(device)
    {
        Create(spec);
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        LOG_INFO("VulkanDescriptorPool::Destructor() ...");
    }

    void VulkanDescriptorPool::Create(const DescriptorPoolSpecification& spec) {}

    void VulkanDescriptorPool::Destroy() {}
}
