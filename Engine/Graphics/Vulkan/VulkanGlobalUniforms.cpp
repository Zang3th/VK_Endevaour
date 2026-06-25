#include "VulkanGlobalUniforms.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanGlobalUniforms::VulkanGlobalUniforms(VulkanContext* context) : m_Context(context)
    {
        CreatePool();
        CreateLayout();
        AllocateBuffers();
        AllocateDescriptorSets();
        WriteDescriptorSets();
    }

    VulkanGlobalUniforms::~VulkanGlobalUniforms()
    {
        LOG_INFO("VulkanGlobalUniforms::Destructor() ...");

        for (u8 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VulkanAllocator::DestroyBuffer(m_BufferAllocs.at(i));
        }
    }

    void VulkanGlobalUniforms::Update(u32 frameIndex, const GlobalUniformData* data)
    {
        ASSERT(frameIndex < FRAMES_IN_FLIGHT, "Given frame index surpasses FRAMES_IN_FLIGHT!");
        ASSERT(data != nullptr, "GlobalUniformData pointer was invalid!");

        auto* dataPtr = VulkanAllocator::MapMemory(m_BufferAllocs.at(frameIndex).Allocation);
        std::memcpy(dataPtr, data, sizeof(GlobalUniformData));
        VulkanAllocator::UnmapMemory(m_BufferAllocs.at(frameIndex).Allocation);
    }

    // ----- Private -----

    void VulkanGlobalUniforms::CreatePool()
    {
        const vk::DescriptorPoolSize poolSize{ .type            = vk::DescriptorType::eUniformBuffer,
                                               .descriptorCount = FRAMES_IN_FLIGHT };

        const DescriptorPoolSpecification spec{
            .Flags     = {},
            .MaxSets   = FRAMES_IN_FLIGHT,
            .PoolSizes = { poolSize },
        };

        m_DescriptorPool = MakeScope<VulkanDescriptorPool>(m_Context->GetDevice()->GetHandle(), spec);
    }

    void VulkanGlobalUniforms::CreateLayout()
    {
        const vk::DescriptorSetLayoutBinding binding{ .binding            = 0,
                                                      .descriptorType     = vk::DescriptorType::eUniformBuffer,
                                                      .descriptorCount    = 1,
                                                      .stageFlags         = vk::ShaderStageFlagBits::eVertex,
                                                      .pImmutableSamplers = nullptr };

        const DescriptorSetLayoutSpecification spec = { .Flags = {}, .Bindings = { binding } };

        m_DescriptorLayout = MakeScope<VulkanDescriptorSetLayout>(m_Context->GetDevice()->GetHandle(), spec);
    }

    void VulkanGlobalUniforms::AllocateBuffers()
    {
        const BufferSpecification spec{ .Size             = sizeof(GlobalUniformData),
                                        .BufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                        .MemoryUsage      = MemoryUsage::eCPUToGPU,
                                        .MemoryFlags      = vk::MemoryPropertyFlagBits::eHostVisible
                                                            | vk::MemoryPropertyFlagBits::eHostCoherent };

        for (u8 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_BufferAllocs.at(i) = VulkanAllocator::AllocateBuffer(spec);
        }
    }

    void VulkanGlobalUniforms::AllocateDescriptorSets()
    {
        std::array<vk::DescriptorSetLayout, FRAMES_IN_FLIGHT> layouts{};
        layouts.fill(m_DescriptorLayout->GetHandle());

        const vk::DescriptorSetAllocateInfo allocInfo{ .descriptorPool     = m_DescriptorPool->GetHandle(),
                                                       .descriptorSetCount = FRAMES_IN_FLIGHT,
                                                       .pSetLayouts        = layouts.data() };

        VK_VERIFY(m_Context->GetDevice()->GetHandle().allocateDescriptorSets(&allocInfo, m_DescriptorSets.data()));
        LOG_INFO("Created descriptor sets ... {}", FRAMES_IN_FLIGHT);
    }

    void VulkanGlobalUniforms::WriteDescriptorSets()
    {
        for (u8 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            const vk::DescriptorBufferInfo bufferInfo{ .buffer = m_BufferAllocs.at(i).Buffer,
                                                       .offset = 0,
                                                       .range  = sizeof(GlobalUniformData) };

            const vk::WriteDescriptorSet descriptorWrite{ .dstSet          = m_DescriptorSets[i],
                                                          .dstBinding      = 0,
                                                          .dstArrayElement = 0,
                                                          .descriptorCount = 1,
                                                          .descriptorType  = vk::DescriptorType::eUniformBuffer,
                                                          .pImageInfo      = nullptr,
                                                          .pBufferInfo     = &bufferInfo };

            m_Context->GetDevice()->GetHandle().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
        }
    }
}
