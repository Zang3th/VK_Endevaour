#pragma once

#include "VulkanContext.hpp"
#include "VulkanShader.hpp"
#include "VulkanModel.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanGlobals.hpp"

namespace Engine
{
    class VulkanRenderer
    {
        public:
            VulkanRenderer();
            ~VulkanRenderer();

            [[nodiscard]] u32 LoadShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path);
            [[nodiscard]] u32 CreateModel(const Mesh* mesh);
            [[nodiscard]] u32 CreatePipeline(u32 vertexID, u32 fragmentID);

            void AssignPipeline(u32 modelID, u32 pipelineID);
            void DrawFrame(u32 pipelineID);
            void WaitForDevice();

        private:
            Scope<VulkanContext> m_Context;
            VulkanSwapchain*     m_Swapchain = nullptr;

            std::array<Scope<VulkanShader>,   MAX_SHADER_COUNT>   m_Shaders;
            std::array<Scope<VulkanModel>,    MAX_MODEL_COUNT>    m_Models;
            std::array<Scope<VulkanPipeline>, MAX_PIPELINE_COUNT> m_Pipelines;
            u32 m_ShaderIndex;
            u32 m_ModelIndex;
            u32 m_PipelineIndex;

            u32 m_DrawcallCount;
    };
}
