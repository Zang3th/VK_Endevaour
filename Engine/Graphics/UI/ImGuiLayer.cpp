#include "Debug/Log.hpp"

#include "Graphics/UI/ImGuiLayer.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    ImGuiLayer::ImGuiLayer(VulkanContext* context) : m_Context(context)
    {
        CreateDescriptorPool();
        Init();
    }

    ImGuiLayer::~ImGuiLayer()
    {
        LOG_INFO("ImGuiLayer::Destructor() ...");
    }

    void ImGuiLayer::BeginFrame() {}

    void ImGuiLayer::EndFrame(vk::CommandBuffer commandBuffer) {}

    // ----- Private -----

    void ImGuiLayer::CreateDescriptorPool()
    {
        constexpr u32 descriptorCount = 1000;

        const DescriptorPoolSpecification spec{
            .MaxSets   = descriptorCount,
            .PoolSizes = { { .type = vk::DescriptorType::eSampler, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eSampledImage, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageImage, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eUniformTexelBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageTexelBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eUniformBufferDynamic, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageBufferDynamic, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eInputAttachment, .descriptorCount = descriptorCount } },
            .Flags     = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet
        };

        m_DescriptorPool = MakeScope<VulkanDescriptorPool>(m_Context->GetDevice()->GetHandle(), spec);
    }

    void ImGuiLayer::Init() {}

    void ImGuiLayer::Shutdown() {}
}
