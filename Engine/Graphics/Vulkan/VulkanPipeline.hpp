#pragma once

#include "VulkanShader.hpp"

namespace Engine
{
    struct PipelineSpecification
    {
        VulkanShader*         VertexShader   = nullptr;
        VulkanShader*         FragmentShader = nullptr;
        vk::PrimitiveTopology Topology       = vk::PrimitiveTopology::eTriangleList;
    };

    class VulkanPipeline
    {
        public:
            explicit VulkanPipeline(const vk::Device& device, const PipelineSpecification& spec);
            ~VulkanPipeline();

            void Bind(vk::CommandBuffer commandBuffer);

        private:
            void CreatePipelineLayout();
            void CreatePipeline();

            vk::Device            m_Device   = nullptr;
            vk::PipelineLayout    m_Layout   = nullptr;
            vk::Pipeline          m_Pipeline = nullptr;
            PipelineSpecification m_Spec;
    };
}
