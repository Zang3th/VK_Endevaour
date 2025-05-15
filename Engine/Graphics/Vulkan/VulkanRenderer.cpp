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

    void VulkanRenderer::DrawFrame(u32 pipelineID)
    {
        // Reset drawcall counter for this frame
        m_DrawcallCount = 0;

        // Get current frame from the swapchain
        auto [renderNextFrame, frame] = m_Context->GetSwapchain()->GetCurrentFrame();
        if(!renderNextFrame)
        {
            return;
        }

        // Start command buffer recording
        vk::CommandBufferBeginInfo cmdBeginInfo{};
        VK_VERIFY(frame.CommandBuffer.begin(&cmdBeginInfo));

        // Set dynamic states
        const vk::Viewport viewport = m_Context->GetSwapchain()->GetViewport();
        const vk::Rect2D   scissor  = m_Context->GetSwapchain()->GetScissor();
        frame.CommandBuffer.setViewport(0, 1, &viewport);
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
                // Bind model
                model->Bind(frame.CommandBuffer);

                // Issue draw call
                // m_CommandBuffer.drawIndexed(model->GetIndexCount(), 1, 0, 0, 0);

                // Increment drawcall counter
                m_DrawcallCount++;
            }
        }

        // End command buffer recording
        VK_VERIFY(frame.CommandBuffer.end());

        // Submit frame (internally increments frame counter)
        m_Context->GetSwapchain()->SubmitFrame(frame);

        // Present image
        // m_Context->GetSwapchain()->PresentNextImage();
    }

    void VulkanRenderer::WaitForDevice()
    {
        m_Context->GetDevice()->WaitForIdle();
    }
}
