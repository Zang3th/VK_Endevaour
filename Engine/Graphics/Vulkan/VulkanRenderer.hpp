#pragma once

#include "Graphics/UI/ImGuiLayer.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"
#include "Graphics/Vulkan/VulkanModel.hpp"
#include "Graphics/Vulkan/VulkanPipeline.hpp"
#include "Graphics/Vulkan/VulkanShader.hpp"

namespace Engine::Graphics
{
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
        void DrawFrame(u32 pipelineID);
        void WaitForDevice();

    private:
        Scope<VulkanContext> m_Context;
        Scope<ImGuiLayer>    m_ImGuiLayer;
        VulkanSwapchain*     m_Swapchain = nullptr;

        std::array<Scope<VulkanShader>, MAX_SHADER_COUNT>     m_Shaders;
        std::array<Scope<VulkanModel>, MAX_MODEL_COUNT>       m_Models;
        std::array<Scope<VulkanPipeline>, MAX_PIPELINE_COUNT> m_Pipelines;
        u32                                                   m_ShaderIndex   = 0;
        u32                                                   m_ModelIndex    = 0;
        u32                                                   m_PipelineIndex = 0;

        u32 m_DrawcallCount = 0;
    };
}
