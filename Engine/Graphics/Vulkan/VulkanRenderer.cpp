#include "VulkanRenderer.hpp"

#include "Debug/Log.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
    {
        m_Context    = MakeScope<VulkanContext>();
        m_ImGuiLayer = MakeScope<ImGuiLayer>(m_Context.get());
        m_Swapchain  = m_Context->GetSwapchain();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        LOG_INFO("VulkanRenderer::Destructor() ...");
    }

    [[nodiscard]] u32 VulkanRenderer::LoadShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path)
    {
        ASSERT(m_ShaderIndex != MAX_SHADER_COUNT, "Reached capacity ... Can't load any more shaders!");

        const u32 currentIndex     = m_ShaderIndex;
        m_Shaders.at(currentIndex) = MakeScope<VulkanShader>(m_Context->GetDevice()->GetHandle(), stage, path);
        m_ShaderIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::CreateModel(const Mesh* mesh)
    {
        ASSERT(m_ModelIndex != MAX_MODEL_COUNT, "Reached capacity ... Can't load any more models!");

        const u32 currentIndex    = m_ModelIndex;
        m_Models.at(currentIndex) = MakeScope<VulkanModel>(m_Context.get(), mesh);
        m_ModelIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::CreatePipeline(u32 vertexID, u32 fragmentID)
    {
        ASSERT(m_PipelineIndex != MAX_PIPELINE_COUNT, "Reached capacity ... Can't create any more pipelines!");

        const u32                   currentIndex = m_PipelineIndex;
        const PipelineSpecification spec{ .VertexShader   = m_Shaders.at(vertexID).get(),
                                          .FragmentShader = m_Shaders.at(fragmentID).get(),
                                          .DepthTest      = vk::False,
                                          .DepthWrite     = vk::False };
        m_Pipelines[currentIndex] = MakeScope<VulkanPipeline>(m_Context.get(), spec);
        m_PipelineIndex++;

        return currentIndex;
    }

    void VulkanRenderer::AssignModelToPipeline(u32 modelID, u32 pipelineID)
    {
        m_Models.at(modelID)->AssignPipeline(pipelineID);
        LOG_INFO("Bound model '{}' to pipeline '{}' ...", modelID, pipelineID);
    }

    void VulkanRenderer::DrawFrame(u32 pipelineID)
    {
        // Reset drawcall counter
        m_DrawcallCount = 0;

        auto frame = m_Swapchain->BeginFrame();
        if (!frame)
        {
            return;
        }

        m_Swapchain->BeginRendering(*frame, glm::vec4(1.0, 0.0, 0.0, 1.0));

        SetDynamicStates(frame->Resources->CommandBuffer, frame->Extent);
        RenderScene(frame->Resources->CommandBuffer, pipelineID);
        RenderUI(frame->Resources->CommandBuffer);

        m_Swapchain->EndRendering(*frame);
        m_Swapchain->SubmitAndPresent(*frame);
    }

    void VulkanRenderer::WaitForDevice()
    {
        m_Context->GetDevice()->WaitForIdle();
    }

    // ----- Private -----

    void VulkanRenderer::SetDynamicStates(vk::CommandBuffer cmdBuffer, vk::Extent2D extent)
    {
        const vk::Viewport viewport = {
            .width = (f32)extent.width, .height = (f32)extent.height, .minDepth = 0.0f, .maxDepth = 1.0f
        };
        cmdBuffer.setViewport(0, 1, &viewport);

        const vk::Rect2D scissor = { .extent = extent };
        cmdBuffer.setScissor(0, 1, &scissor);

        cmdBuffer.setPolygonModeEXT(vk::PolygonMode::eFill, m_Context->GetLoader());
        cmdBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
        cmdBuffer.setCullMode(vk::CullModeFlagBits::eBack);
        cmdBuffer.setFrontFace(vk::FrontFace::eClockwise);
    }

    void VulkanRenderer::RenderScene(vk::CommandBuffer cmdBuffer, u32 pipelineID)
    {
        // Bind pipeline
        m_Pipelines.at(pipelineID)->Bind(cmdBuffer);

        // Draw all models assigned to this pipeline
        for (u32 i = 0; i < m_ModelIndex; i++)
        {
            // Get model
            const VulkanModel* model = m_Models.at(i).get();

            // Check for pipeline
            if (model->GetPipelineID() == pipelineID)
            {
                // Bind, draw, increment
                model->Bind(cmdBuffer);
                cmdBuffer.drawIndexed(model->GetIndexCount(), 1, 0, 0, 0);
                m_DrawcallCount++;
            }
        }
    }

    void VulkanRenderer::RenderUI(vk::CommandBuffer cmdBuffer)
    {
        m_ImGuiLayer->BeginFrame();

        // TODO: Render ProfilerPanel

        m_ImGuiLayer->RenderFrame(cmdBuffer);
    }
}
