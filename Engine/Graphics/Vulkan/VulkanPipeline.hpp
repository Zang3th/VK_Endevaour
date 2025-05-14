#pragma once

#include "VulkanShader.hpp"
#include "VulkanContext.hpp"

namespace Engine
{
    struct PipelineSpecification
    {
        VulkanShader* VertexShader   = nullptr;
        VulkanShader* FragmentShader = nullptr;
        vk::Bool32    DepthTest      = vk::True;
        vk::Bool32    DepthWrite     = vk::True;
        vk::CompareOp DepthOperator  = vk::CompareOp::eLessOrEqual;
    };

    class VulkanPipeline
    {
        public:
            explicit VulkanPipeline(VulkanContext* context, const PipelineSpecification& spec);
            ~VulkanPipeline();

            void Bind(vk::CommandBuffer commandBuffer);

        private:
            void CreatePipelineLayout();
            void CreatePipeline();

            VulkanContext*        m_Context  = nullptr;
            vk::PipelineLayout    m_Layout   = nullptr;
            vk::Pipeline          m_Pipeline = nullptr;
            PipelineSpecification m_Spec;
    };
}
