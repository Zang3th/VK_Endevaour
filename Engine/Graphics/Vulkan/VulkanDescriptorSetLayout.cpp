#include "VulkanDescriptorSetLayout.hpp"

#include "Core/Types.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(vk::Device                              device,
                                                         const DescriptorSetLayoutSpecification& spec)
        : m_Device(device)
    {
        Create(spec);
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        LOG_INFO("VulkanDescriptorSetLayout::Destructor() ...");
        m_Device.destroyDescriptorSetLayout(m_Layout);
    }

    // ----- Private -----

    void VulkanDescriptorSetLayout::Create(const DescriptorSetLayoutSpecification& spec)
    {
        const vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{ .flags        = spec.Flags,
                                                                  .bindingCount = (u32)(spec.Bindings.size()),
                                                                  .pBindings    = spec.Bindings.data() };

        VK_VERIFY(m_Device.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &m_Layout));
        LOG_INFO("Created descriptor set layout ... (Count: {})", spec.Bindings.size());
    }
}
