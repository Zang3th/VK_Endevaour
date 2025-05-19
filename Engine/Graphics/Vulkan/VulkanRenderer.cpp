#include "VulkanRenderer.hpp"

#include "Debug/Log.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
        : m_ShaderIndex(0), m_ModelIndex(0), m_PipelineIndex(0), m_DrawcallCount(0)
    {
        m_Context   = MakeScope<VulkanContext>();
        m_Swapchain = m_Context->GetSwapchain();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        LOG_INFO("VulkanRenderer::Destructor() ...");
    }

    [[nodiscard]] u32 VulkanRenderer::LoadShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path)
    {
        ASSERT(m_ShaderIndex != MAX_SHADER_COUNT, "Reached capacity ... Can't load any more shaders!");

        u32 currentIndex = m_ShaderIndex;
        m_Shaders.at(currentIndex) = MakeScope<VulkanShader>(m_Context->GetDevice()->GetHandle(), stage, path);
        m_ShaderIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::CreateModel(const Mesh* mesh)
    {
        ASSERT(m_ModelIndex != MAX_MODEL_COUNT, "Reached capacity ... Can't load any more models!");

        u32 currentIndex = m_ModelIndex;
        m_Models.at(currentIndex) = MakeScope<VulkanModel>(m_Context.get(), mesh);
        m_ModelIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::CreatePipeline(u32 vertexID, u32 fragmentID)
    {
        ASSERT(m_PipelineIndex != MAX_PIPELINE_COUNT, "Reached capacity ... Can't create any more pipelines!");

        u32 currentIndex = m_PipelineIndex;
        PipelineSpecification spec
        {
            .VertexShader   = m_Shaders.at(vertexID).get(),
            .FragmentShader = m_Shaders.at(fragmentID).get(),
            .DepthTest      = vk::False,
            .DepthWrite     = vk::False
        };
        m_Pipelines[currentIndex] = MakeScope<VulkanPipeline>(m_Context.get(), spec);
        m_PipelineIndex++;

        return currentIndex;
    }

    void VulkanRenderer::AssignPipeline(u32 modelID, u32 pipelineID)
    {
        m_Models.at(modelID)->AssignPipeline(pipelineID);
        LOG_INFO("Bound model '{}' to pipeline '{}' ...", modelID, pipelineID);
    }

    void VulkanRenderer::DrawFrame(u32 pipelineID)
    {
        // Reset drawcall counter
        m_DrawcallCount = 0;

        // Get next frame from swapchain
        auto [renderNextFrame, frame] = m_Context->GetSwapchain()->GetNextFrame();
        if(!renderNextFrame)
        {
            return; // Swapchain was probably recreated
        }

        // Get swapchain properties for this frame
        SwapchainProperties properties = m_Swapchain->GetProperties();

        // Begin frame
        frame.Begin(m_Swapchain->GetImageAt(frame.ImageIndex), properties.Extent);

        // Set dynamic states
        vk::Viewport viewport =
        {
            .width  = (float)properties.Extent.width,
            .height = (float)properties.Extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        frame.CommandBuffer.setViewport(0, 1, &viewport);

        vk::Rect2D scissor = { .extent = properties.Extent };
        frame.CommandBuffer.setScissor(0, 1, &scissor);

        frame.CommandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill, m_Context->GetLoader());
        frame.CommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
        frame.CommandBuffer.setCullMode(vk::CullModeFlagBits::eBack);
        frame.CommandBuffer.setFrontFace(vk::FrontFace::eClockwise);

        // Bind pipeline
        m_Pipelines.at(pipelineID)->Bind(frame.CommandBuffer);

        // Draw all models assigned to this pipeline
        for(u32 i = 0; i < m_ModelIndex; i++)
        {
            // Get model
            const VulkanModel* model = m_Models.at(i).get();

            // Check for pipeline
            if(model->GetPipelineID() == pipelineID)
            {
                // Bind, draw, increment
                model->Bind(frame.CommandBuffer);
                frame.CommandBuffer.drawIndexed(model->GetIndexCount(), 1, 0, 0, 0);
                m_DrawcallCount++;
            }
        }

        // End frame
        frame.End(m_Swapchain->GetImageAt(frame.ImageIndex));

        // Submit, present, advance
        m_Context->GetSwapchain()->SubmitFrame(frame);
        m_Context->GetSwapchain()->PresentFrame(frame);
        m_Context->GetSwapchain()->AdvanceFrameCount();
    }

    void VulkanRenderer::WaitForDevice()
    {
        m_Context->GetDevice()->WaitForIdle();
    }
}
