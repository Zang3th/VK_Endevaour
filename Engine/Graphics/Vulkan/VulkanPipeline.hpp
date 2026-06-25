#pragma once

#include "Graphics/Vulkan/VulkanContext.hpp"
#include "Graphics/Vulkan/VulkanShader.hpp"

namespace Engine::Graphics
{
    struct PipelineSpecification
    {
        VulkanShader*           VertexShader             = nullptr;
        VulkanShader*           FragmentShader           = nullptr;
        vk::DescriptorSetLayout DescriptorSetLayout      = nullptr;
        u32                     DescriptorSetLayoutCount = DESCRIPTOR_SET_COUNT;
        vk::Bool32              DepthTest                = vk::False;
        vk::Bool32              DepthWrite               = vk::False;
        vk::CompareOp           DepthOperator            = vk::CompareOp::eLessOrEqual;
    };

    class VulkanPipeline
    {
    public:
        explicit VulkanPipeline(VulkanContext* context, const PipelineSpecification& spec);
        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline&)            = delete;
        VulkanPipeline& operator=(const VulkanPipeline&) = delete;

        [[nodiscard]] vk::PipelineLayout GetLayout() const { return m_Layout; }

        void Bind(vk::CommandBuffer commandBuffer) const;

    private:
        void CreatePipelineLayout();
        void CreatePipeline();

        VulkanContext*        m_Context  = nullptr;
        vk::PipelineLayout    m_Layout   = nullptr;
        vk::Pipeline          m_Pipeline = nullptr;
        PipelineSpecification m_Spec;
    };
}
