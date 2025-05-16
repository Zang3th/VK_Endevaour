#pragma once

#include "VulkanAllocator.hpp"
#include "VulkanContext.hpp"

#include "Graphics/Mesh.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine
{
    class VulkanModel
    {
        public:
            VulkanModel(VulkanContext* context, const Mesh* mesh);
            ~VulkanModel();

            void Bind(vk::CommandBuffer commandBuffer) const;

            [[nodiscard]] vk::Buffer GetVertexBuffer() const { return m_VertexBuffer;         };
            [[nodiscard]] vk::Buffer GetIndexBuffer()  const { return m_IndexBuffer;          };
            [[nodiscard]] u32        GetIndexCount()   const { return m_Mesh->Indices.size(); };
            [[nodiscard]] u32        GetPipelineID()   const { return m_PipelineID;           };

            void AssignPipeline(u32 id) { m_PipelineID = id; };

        private:
            void CreateVertexBuffer();
            void CreateIndexBuffer();

            VulkanContext*       m_Context          = nullptr;
            vk::Buffer           m_VertexBuffer     = nullptr;
            VmaAllocation        m_VertexAllocation = nullptr;
            vk::Buffer           m_IndexBuffer      = nullptr;
            VmaAllocation        m_IndexAllocation  = nullptr;
            const Mesh*          m_Mesh             = nullptr;
            u32                  m_PipelineID       = UINT32_MAX;
    };
}
