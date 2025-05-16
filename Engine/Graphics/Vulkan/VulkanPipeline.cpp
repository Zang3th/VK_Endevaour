#include "VulkanPipeline.hpp"
#include "VulkanAssert.hpp"

#include "Graphics/Mesh.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanPipeline::VulkanPipeline(VulkanContext* context, const PipelineSpecification& spec)
        : m_Context(context), m_Spec(spec)
    {
        CreatePipelineLayout();
        CreatePipeline();
    }

    VulkanPipeline::~VulkanPipeline()
    {
        LOG_INFO("VulkanPipeline::Destructor() ...");
        m_Context->GetDevice()->GetHandle().destroyPipelineLayout(m_Layout);
        m_Context->GetDevice()->GetHandle().destroyPipeline(m_Pipeline);
    }

    void VulkanPipeline::Bind(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
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

        VK_VERIFY(m_Context->GetDevice()->GetHandle().createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_Layout));
        LOG_INFO("Created pipeline layout ...");
    }

    void VulkanPipeline::CreatePipeline()
    {
        // ----- Construct the different states making up the pipeline (using dynamic rendering) -----

        // Pipeline rendering info (for dynamic rendering)
        vk::Format format = m_Context->GetSwapchain()->GetSurfaceFormat().format;
        vk::PipelineRenderingCreateInfo renderingInfo
        {
            .colorAttachmentCount    = 1,
            .pColorAttachmentFormats = &format
        };

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
            .depthBiasEnable         = vk::False,
            .lineWidth               = 1.0f
        };

        // Multisampling state
        vk::PipelineMultisampleStateCreateInfo multisampleState
        {
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False
        };

        // Depth stencil state
        vk::PipelineDepthStencilStateCreateInfo depthStencilState
        {
            .depthTestEnable       = m_Spec.DepthTest,
            .depthWriteEnable      = m_Spec.DepthWrite,
            .depthCompareOp        = m_Spec.DepthOperator,
            .depthBoundsTestEnable = vk::False,
            .stencilTestEnable     = vk::False
        };

        // Disable blending (new fragment colors will just get passed through to the framebuffer)
        vk::PipelineColorBlendAttachmentState blendAttachment
        {
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        // Color blend state
        vk::PipelineColorBlendStateCreateInfo colorBlendState
        {
            .attachmentCount = 1,
            .pAttachments    = &blendAttachment
        };

        // Specify dynamic states (can be changed without recreating the whole pipeline)
        std::vector<vk::DynamicState> dynamicStates =
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
            vk::DynamicState::ePolygonModeEXT,
            vk::DynamicState::ePrimitiveTopology,
            vk::DynamicState::eCullMode,
            vk::DynamicState::eFrontFace
        };
        vk::PipelineDynamicStateCreateInfo dynamicState
        {
            .dynamicStateCount = (u32)(dynamicStates.size()),
            .pDynamicStates    = dynamicStates.data()
        };

        // ----- Finally: Create the pipeline -----
        vk::GraphicsPipelineCreateInfo pipelineInfo
        {
            .pNext               = &renderingInfo,
            .stageCount          = 2,
            .pStages             = shaderStages,
            .pVertexInputState   = &vertexState,
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState      = &viewportState,
            .pRasterizationState = &rasterizerState,
            .pMultisampleState   = &multisampleState,
            .pDepthStencilState  = &depthStencilState,
            .pColorBlendState    = &colorBlendState,
            .pDynamicState       = &dynamicState,
            .layout              = m_Layout
        };

        auto [res, pipeline] = m_Context->GetDevice()->GetHandle().createGraphicsPipeline(nullptr, pipelineInfo);
        VK_VERIFY(res);
        m_Pipeline = pipeline;
        LOG_INFO("Created graphics pipeline ...");
    }
}
