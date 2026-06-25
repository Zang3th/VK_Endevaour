#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

namespace Engine::Graphics
{
    struct DescriptorSetLayoutSpecification
    {
        vk::DescriptorSetLayoutCreateFlags          Flags;
        std::vector<vk::DescriptorSetLayoutBinding> Bindings;
    };

    class VulkanDescriptorSetLayout
    {
    public:
        VulkanDescriptorSetLayout(vk::Device device, const DescriptorSetLayoutSpecification& spec);
        ~VulkanDescriptorSetLayout();

        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&)            = delete;
        VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

        [[nodiscard]] vk::DescriptorSetLayout GetHandle() const { return m_Layout; }

    private:
        void Create(const DescriptorSetLayoutSpecification& spec);

        vk::Device              m_Device = nullptr;
        vk::DescriptorSetLayout m_Layout = nullptr;
    };
}
