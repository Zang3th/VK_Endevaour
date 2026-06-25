#pragma once

#include "Core/Timer.hpp"

#include "Graphics/UI/ImGuiLayer.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"
#include "Graphics/Vulkan/VulkanGlobalUniforms.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"
#include "Graphics/Vulkan/VulkanModel.hpp"
#include "Graphics/Vulkan/VulkanPipeline.hpp"
#include "Graphics/Vulkan/VulkanShader.hpp"

namespace Engine::Graphics
{
    struct RenderPacket
    {
        std::optional<SwapchainFrame> Frame;
        u32                           PipelineID = 0;

        [[nodiscard]] bool IsValid() const { return Frame.has_value(); }
    };

    class VulkanRenderer
    {
    public:
        VulkanRenderer();
        ~VulkanRenderer();

        VulkanRenderer(const VulkanRenderer&)            = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        [[nodiscard]] u32 LoadShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path);
        [[nodiscard]] u32 CreateModel(const Mesh* mesh);
        [[nodiscard]] u32 CreatePipeline(u32 vertexID, u32 fragmentID);

        void AssignModelToPipeline(u32 modelID, u32 pipelineID);

        [[nodiscard]] RenderPacket BeginFrame(u32 pipelineID);
        void                       DrawFrame(RenderPacket renderPacket, const Core::FrameTiming& frameTiming);

        void WaitForDevice();

    private:
        void SetDynamicStates(vk::CommandBuffer cmdBuffer, vk::Extent2D extent);
        void UpdateGlobalUniforms(vk::Extent2D extent, u32 frameIndex, const Core::FrameTiming& frameTiming);
        void RenderScene(vk::CommandBuffer cmdBuffer, u32 pipelineID, u32 frameIndex);
        void RenderUI(vk::CommandBuffer cmdBuffer, const Core::FrameTiming& frameTiming);

        // Vulkan context, UI and swapchain shortcut for quick access
        Scope<VulkanContext> m_Context;
        Scope<ImGuiLayer>    m_ImGuiLayer;
        VulkanSwapchain*     m_Swapchain = nullptr;

        // Uniform stuff
        Scope<VulkanGlobalUniforms> m_VulkanGlobalUniforms; // Should live longer than the pipeline
        GlobalUniformData           m_GlobalUniformData;

        // Shader, Models, Pipelines
        std::array<Scope<VulkanShader>, MAX_SHADER_COUNT>     m_Shaders;
        std::array<Scope<VulkanModel>, MAX_MODEL_COUNT>       m_Models;
        std::array<Scope<VulkanPipeline>, MAX_PIPELINE_COUNT> m_Pipelines;

        u32 m_ShaderIndex   = 0;
        u32 m_ModelIndex    = 0;
        u32 m_PipelineIndex = 0;
        u32 m_DrawcallCount = 0;
    };
}
