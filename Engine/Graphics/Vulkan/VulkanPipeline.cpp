#include "VulkanPipeline.hpp"
#include "VulkanAssert.hpp"

#include "Graphics/Mesh.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanPipeline::VulkanPipeline(const vk::Device& device, const PipelineSpecification& spec)
        : m_Device(device), m_Spec(spec)
    {
        CreatePipelineLayout();
        CreatePipeline();
    }

    VulkanPipeline::~VulkanPipeline()
    {
        m_Device.destroyPipelineLayout(m_Layout);
    }

    void VulkanPipeline::Bind(vk::CommandBuffer commandBuffer)
    {
        // TODO: Implement
    }

    // ----- Private -----

    void VulkanPipeline::CreatePipelineLayout()
    {
        // TODO: Add descriptor sets for textures and uniform buffers (should come from shader)
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo
        {
            .setLayoutCount         = 0,
            .pSetLayouts            = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr
        };

        VK_VERIFY(m_Device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_Layout));
        LOG_INFO("Created pipeline layout ...");
    }

    void VulkanPipeline::CreatePipeline()
    {
        // ----- Construct the different states making up the pipeline -----

        // Shader stages
        vk::PipelineShaderStageCreateInfo shaderStages[] =
        {
            m_Spec.VertexShader->GetPipelineShaderStageCreateInfo(),
            m_Spec.FragmentShader->GetPipelineShaderStageCreateInfo()
        };

        // Vertex input state
        auto bindingDescription    = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        vk::PipelineVertexInputStateCreateInfo vertexState
        {
            .vertexBindingDescriptionCount   = 1,
            .pVertexBindingDescriptions      = &bindingDescription,
            .vertexAttributeDescriptionCount = (u32)(attributeDescriptions.size()),
            .pVertexAttributeDescriptions    = attributeDescriptions.data()
        };

        // Input assembly state
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState
        {
            .topology               = m_Spec.Topology,
            .primitiveRestartEnable = vk::False
        };

        // Viewport state
        vk::PipelineViewportStateCreateInfo viewportState
        {
            .viewportCount = 1,
            .scissorCount  = 1
        };

        // Rasterization state
        vk::PipelineRasterizationStateCreateInfo rasterizerState
        {
            .depthClampEnable        = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode             = vk::PolygonMode::eFill,
            .cullMode                = vk::CullModeFlagBits::eBack,
            .frontFace               = vk::FrontFace::eClockwise,
            .depthBiasEnable         = vk::False,
            .lineWidth               = 1.0f
        };

        // Multisampling state
        vk::PipelineMultisampleStateCreateInfo multisampleState
        {
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False
        };

        // TODO: Add depth stencil state

        // Specify dynamic states (can be changed without recreating the whole pipeline)
        std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicState
        {
            .dynamicStateCount = (u32)(dynamicStates.size()),
            .pDynamicStates    = dynamicStates.data()
        };

        // ----- Finally: Create the pipeline -----
        vk::GraphicsPipelineCreateInfo pipelineInfo
        {
            .stageCount          = 2,
            .pStages             = shaderStages,
            .pVertexInputState   = &vertexState,
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState      = &viewportState,
            .pRasterizationState = &rasterizerState,
            .pMultisampleState   = &multisampleState,
            // .pDepthStencilState = nullptr,
            .pDynamicState       = &dynamicState,
            .layout              = m_Layout
        };

        auto [res, pipeline] = m_Device.createGraphicsPipeline(nullptr, pipelineInfo);
        VK_VERIFY(res);
        m_Pipeline = pipeline;
        LOG_INFO("Created graphics pipeline ...");
    }
}
