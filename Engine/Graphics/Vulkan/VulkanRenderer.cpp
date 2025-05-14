#include "VulkanRenderer.hpp"

#include "Debug/Log.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
        : m_ShaderIndex(0), m_ModelIndex(0), m_PipelineIndex(0)
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
        // Get graphics command buffer from the swapchain and begin recording
        // commandBuffer = BeginFrame();

        // For every model

            // Bind pipeline (if not already bound)

            // Bind model

            // Issue draw commands

            // Increment drawcall counter

        // Submit command buffer and present image
        // EndFrame();
    }
}
