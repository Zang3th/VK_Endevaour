#pragma once

#include "VulkanAllocator.hpp"
#include "VulkanContext.hpp"

#include "Graphics/Mesh.hpp"

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Engine
{
    class VulkanModel
    {
        public:
            VulkanModel(VulkanContext* context, const std::filesystem::path& path);
            ~VulkanModel();

            [[nodiscard]] vk::Buffer GetVertexBuffer() const { return m_VertexBuffer;        };
            [[nodiscard]] vk::Buffer GetIndexBuffer()  const { return m_IndexBuffer;         };
            [[nodiscard]] u32        GetIndexCount()   const { return m_Mesh.Indices.size(); };

        private:
            void CreateVertexBuffer();
            void CreateIndexBuffer();

            VulkanContext*       m_Context          = nullptr;
            vk::Buffer           m_VertexBuffer     = nullptr;
            VmaAllocation        m_VertexAllocation = nullptr;
            vk::Buffer           m_IndexBuffer      = nullptr;
            VmaAllocation        m_IndexAllocation  = nullptr;
            Mesh                 m_Mesh;
    };
}
