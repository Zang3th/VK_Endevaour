#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    struct DescriptorPoolSpecification
    {
        u32                                 MaxSets = 0;
        std::vector<vk::DescriptorPoolSize> PoolSizes;
        vk::DescriptorPoolCreateFlags       Flags;
    };

    class VulkanDescriptorPool
    {
    public:
        VulkanDescriptorPool(vk::Device device, const DescriptorPoolSpecification& spec);
        ~VulkanDescriptorPool();

        VulkanDescriptorPool(const VulkanDescriptorPool&)            = delete;
        VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

        [[nodiscard]] vk::DescriptorPool GetHandle() const;

    private:
        void Create(const DescriptorPoolSpecification& spec);
        void Destroy();

        vk::Device         m_Device;
        vk::DescriptorPool m_Pool;
    };
}
