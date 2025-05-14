#include "VulkanRenderer.hpp"
#include "VulkanAssert.hpp"

#include "Debug/Log.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
        : m_ShaderIndex(0), m_ModelIndex(0), m_PipelineIndex(0), m_DrawcallCount(0)
    {
        m_Context = MakeScope<VulkanContext>();
    }

    [[nodiscard]] u32 VulkanRenderer::LoadShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path)
    {
        ASSERT(m_ShaderIndex != MAX_SHADER_COUNT, "Reached capacity ... Can't load any more shaders!");

        u32 currentIndex = m_ShaderIndex;
        m_Shaders.at(currentIndex) = MakeScope<VulkanShader>(m_Context->GetDevice()->GetHandle(), stage, path);
        m_ShaderIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::LoadModel(const std::filesystem::path& path)
    {
        ASSERT(m_ModelIndex != MAX_MODEL_COUNT, "Reached capacity ... Can't load any more models!");

        u32 currentIndex = m_ModelIndex;
        m_Models.at(currentIndex) = MakeScope<VulkanModel>(m_Context.get(), path);
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

    void VulkanRenderer::BeginFrame()
    {
        // Reset drawcall counter for this frame
        m_DrawcallCount = 0;

        // Get graphics command buffer from the swapchain and begin recording
        // m_CommandBuffer = m_Context->GetSwapchain()->BeginFrame();

        // Set dynamic states
        const vk::Viewport viewport = m_Context->GetSwapchain()->GetViewport();
        const vk::Rect2D   scissor  = m_Context->GetSwapchain()->GetScissor();
        m_CommandBuffer.setViewport(0, 1, &viewport);
        m_CommandBuffer.setScissor(0, 1, &scissor);
        m_CommandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill, m_Context->GetLoader());
        m_CommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
        m_CommandBuffer.setCullMode(vk::CullModeFlagBits::eBack);
        m_CommandBuffer.setFrontFace(vk::FrontFace::eClockwise);
    }

    void VulkanRenderer::DrawFrame(u32 pipelineID)
    {
        // Bind pipeline
        // m_Pipelines.at(pipelineID)->Bind(m_CommandBuffer);

        // Draw all models assigned to this pipeline
        for(u32 i = 0; i < m_ModelIndex; i++)
        {
            // Get model
            const VulkanModel* model = m_Models.at(i).get();

            // Check for pipeline
            if(model->GetPipelineID() == pipelineID)
            {
                // Bind model
                // model->Bind(m_CommandBuffer);

                // Issue draw call
                // m_CommandBuffer.drawIndexed(model->GetIndexCount(), 1, 0, 0, 0);

                // Increment drawcall counter
                m_DrawcallCount++;
            }
        }
    }

    void VulkanRenderer::EndFrame()
    {
        // Submit command buffer
        // m_Context->GetSwapchain()->EndFrame(m_CommandBuffer);

        // Present image
        // m_Context->GetSwapchain()->PresentImage();
    }

    void VulkanRenderer::WaitForDevice()
    {
        VK_VERIFY(m_Context->GetDevice()->GetHandle().waitIdle());
    }
}
