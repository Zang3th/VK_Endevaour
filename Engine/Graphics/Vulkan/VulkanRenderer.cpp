#include "VulkanRenderer.hpp"

#include "Debug/Log.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
    {
        m_Context              = MakeScope<VulkanContext>();
        m_ImGuiLayer           = MakeScope<ImGuiLayer>(m_Context.get());
        m_VulkanGlobalUniforms = MakeScope<VulkanGlobalUniforms>(m_Context.get());
        m_Swapchain            = m_Context->GetSwapchain();
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
        const PipelineSpecification spec{ .VertexShader        = m_Shaders.at(vertexID).get(),
                                          .FragmentShader      = m_Shaders.at(fragmentID).get(),
                                          .DescriptorSetLayout = m_VulkanGlobalUniforms->GetLayout()->GetHandle() };

        m_Pipelines[currentIndex] = MakeScope<VulkanPipeline>(m_Context.get(), spec);
        m_PipelineIndex++;

        return currentIndex;
    }

    void VulkanRenderer::AssignModelToPipeline(u32 modelID, u32 pipelineID)
    {
        m_Models.at(modelID)->AssignPipeline(pipelineID);
        LOG_INFO("Bound model '{}' to pipeline '{}' ...", modelID, pipelineID);
    }

    [[nodiscard]] RenderPacket VulkanRenderer::BeginFrame(u32 pipelineID)
    {
        return { .Frame = m_Swapchain->BeginFrame(), .PipelineID = pipelineID };
    }

    void VulkanRenderer::DrawFrame(RenderPacket renderPacket, const Core::FrameTiming& frameTiming)
    {
        ASSERT(renderPacket.Frame.has_value(), "Application should only commit valid frames!");
        const SwapchainFrame frame = *renderPacket.Frame;

        // Reset drawcall counter
        m_DrawcallCount = 0;

        m_Swapchain->BeginRendering(frame, glm::vec4(1.0, 0.0, 1.0, 1.0));

        SetDynamicStates(frame.Resources->CommandBuffer, frame.Extent);
        UpdateGlobalUniforms(frame.Extent, frame.FrameIndex, frameTiming);
        RenderScene(frame.Resources->CommandBuffer, renderPacket.PipelineID, frame.FrameIndex);
        RenderUI(frame.Resources->CommandBuffer, frameTiming);

        m_Swapchain->EndRendering(frame);
        m_Swapchain->SubmitAndPresent(frame);
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
        cmdBuffer.setFrontFace(vk::FrontFace::eCounterClockwise);
    }

    void VulkanRenderer::UpdateGlobalUniforms(vk::Extent2D extent, u32 frameIndex, const Core::FrameTiming& frameTiming)
    {
        // Update uniform data (later with real camera information)
        m_GlobalUniformData.Model = glm::rotate(
            glm::mat4(1.0f), (f32)frameTiming.TotalSeconds * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_GlobalUniformData.View =
            glm::lookAt(glm::vec3(0.0f, 15.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_GlobalUniformData.Projection =
            glm::perspective(glm::radians(45.0f), (f32)extent.width / (f32)extent.height, 0.1f, 100.0f);
        m_GlobalUniformData.Projection[1][1] *= -1; // Flip Y-Coordinate of clip coordinates because of legacy OpenGL

        // Inform global uniforms that the data has changed
        m_VulkanGlobalUniforms->Update(frameIndex, &m_GlobalUniformData);
    }

    void VulkanRenderer::RenderScene(vk::CommandBuffer cmdBuffer, u32 pipelineID, u32 frameIndex)
    {
        // Bind pipeline
        m_Pipelines.at(pipelineID)->Bind(cmdBuffer);

        // Bind global descriptor set for current frame
        cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     m_Pipelines.at(pipelineID)->GetLayout(),
                                     0,
                                     DESCRIPTOR_SET_COUNT,
                                     m_VulkanGlobalUniforms->GetDescriptorSet(frameIndex),
                                     0,
                                     nullptr);

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

    void VulkanRenderer::RenderUI(vk::CommandBuffer cmdBuffer, [[maybe_unused]] const Core::FrameTiming& frameTiming)
    {
        m_ImGuiLayer->BeginFrame();

        // TODO: Render ProfilerPanel

        // TODO: Visualize frame timing

        m_ImGuiLayer->RenderFrame(cmdBuffer);
    }
}
