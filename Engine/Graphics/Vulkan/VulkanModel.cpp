#include "VulkanModel.hpp"

#include "Debug/Log.hpp"

#include "Graphics/ObjLoader.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanModel::VulkanModel(VulkanContext* context, const std::filesystem::path& path)
        : m_Context(context)
    {
        m_Mesh = ObjLoader::LoadMeshFromFile(path);
        CreateVertexBuffer();
        CreateIndexBuffer();
    }

    VulkanModel::~VulkanModel()
    {
        VulkanAllocator::DestroyBuffer(m_VertexBuffer, m_VertexAllocation);
        VulkanAllocator::DestroyBuffer(m_IndexBuffer, m_IndexAllocation);
    }

    void VulkanModel::Bind(vk::CommandBuffer commandBuffer) const
    {
        vk::DeviceSize offset = 0;
        commandBuffer.bindVertexBuffers(0, 1, &m_VertexBuffer, &offset);
        commandBuffer.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint32);
    }

    // ----- Private -----

    void VulkanModel::CreateVertexBuffer()
    {
        ASSERT(!m_Mesh.Vertices.empty(), "Model has no vertex data!");

        // Create vertex buffer
        auto [vertexBuffer, vertexAllocation] = VulkanAllocator::AllocateBuffer
        (
            m_Mesh.GetVerticeSize(),
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            MemoryUsage::eGPUOnly
        );
        m_VertexBuffer     = vertexBuffer;
        m_VertexAllocation = vertexAllocation;

        // Create staging buffer
        auto [stagingBuffer, stagingAllocation] = VulkanAllocator::AllocateBuffer
        (
            m_Mesh.GetVerticeSize(),
            vk::BufferUsageFlagBits::eTransferSrc,
            MemoryUsage::eCPUOnly
        );

        // Fill out staging buffer
        void* dataPtr = VulkanAllocator::MapMemory(stagingAllocation);
        std::memcpy(dataPtr, m_Mesh.Vertices.data(), m_Mesh.GetVerticeSize());
        VulkanAllocator::UnmapMemory(stagingAllocation);

        // Transfer data from CPU to GPU
        m_Context->CopyBuffer(stagingBuffer, m_VertexBuffer, m_Mesh.GetVerticeSize());

        // Destroy staging buffer
        VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);

        LOG_INFO("Created and uploaded vertex buffer ...");
    }

    void VulkanModel::CreateIndexBuffer()
    {
        ASSERT(!m_Mesh.Indices.empty(), "Model has no index data!");

        // Create index buffer
        auto [indexBuffer, indexAllocation] = VulkanAllocator::AllocateBuffer
        (
            m_Mesh.GetIndiceSize(),
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            MemoryUsage::eGPUOnly
        );
        m_IndexBuffer     = indexBuffer;
        m_IndexAllocation = indexAllocation;

        // Create staging buffer
        auto [stagingBuffer, stagingAllocation] = VulkanAllocator::AllocateBuffer
        (
            m_Mesh.GetIndiceSize(),
            vk::BufferUsageFlagBits::eTransferSrc,
            MemoryUsage::eCPUOnly
        );

        // Fill out staging buffer
        void* dataPtr = VulkanAllocator::MapMemory(stagingAllocation);
        std::memcpy(dataPtr, m_Mesh.Indices.data(), m_Mesh.GetIndiceSize());
        VulkanAllocator::UnmapMemory(stagingAllocation);

        // Transfer data from CPU to GPU
        m_Context->CopyBuffer(stagingBuffer, m_IndexBuffer, m_Mesh.GetIndiceSize());

        // Destroy staging buffer
        VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);

        LOG_INFO("Created and uploaded index buffer ...");
    }
}
