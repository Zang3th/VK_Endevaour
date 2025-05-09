#pragma once

#include "Graphics/Mesh.hpp"

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Engine
{
    class VulkanModel
    {
        public:
            VulkanModel(const vk::Device& device, const std::filesystem::path& path);
            ~VulkanModel();

            [[nodiscard]] vk::Buffer GetVertexBuffer() const { return m_VertexBuffer;        };
            [[nodiscard]] vk::Buffer GetIndexBuffer()  const { return m_IndexBuffer;         };
            [[nodiscard]] u32        GetIndexCount()   const { return m_Mesh.Indices.size(); };

        private:
            void CreateVertexBuffer();
            void CreateIndexBuffer();

            vk::Device m_Device       = nullptr;
            vk::Buffer m_VertexBuffer = nullptr;
            vk::Buffer m_IndexBuffer  = nullptr;
            Mesh       m_Mesh;
    };
}
