#include "VulkanModel.hpp"

#include "Debug/Log.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanModel::VulkanModel(VulkanContext* context, const Mesh* mesh) : m_Context(context), m_Mesh(mesh)
    {
        CreateVertexBuffer();
        CreateIndexBuffer();
    }

    VulkanModel::~VulkanModel()
    {
        LOG_INFO("VulkanModel::Destructor() ...");
        VulkanAllocator::DestroyBuffer(m_VertexBufferAlloc);
        VulkanAllocator::DestroyBuffer(m_IndexBufferAlloc);
    }

    void VulkanModel::Bind(vk::CommandBuffer commandBuffer) const
    {
        const vk::DeviceSize offset = 0;
        commandBuffer.bindVertexBuffers(0, 1, &m_VertexBufferAlloc.Buffer, &offset);
        commandBuffer.bindIndexBuffer(m_IndexBufferAlloc.Buffer, 0, vk::IndexType::eUint32);
    }

    // ----- Private -----

    void VulkanModel::CreateVertexBuffer()
    {
        ASSERT(!m_Mesh->Vertices.empty(), "Model has no vertex data!");

        // Create vertex buffer
        const BufferSpecification vboSpec{ .Size             = m_Mesh->GetVerticeSize(),
                                           .BufferUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer
                                                               | vk::BufferUsageFlagBits::eTransferDst,
                                           .MemoryUsage      = MemoryUsage::eGPUOnly,
                                           .MemoryFlags      = vk::MemoryPropertyFlagBits::eDeviceLocal };
        m_VertexBufferAlloc = VulkanAllocator::AllocateBuffer(vboSpec);

        // Create staging buffer
        const BufferSpecification stagingSpec{ .Size             = m_Mesh->GetVerticeSize(),
                                               .BufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
                                               .MemoryUsage      = MemoryUsage::eCPUOnly,
                                               .MemoryFlags      = vk::MemoryPropertyFlagBits::eHostVisible
                                                                   | vk::MemoryPropertyFlagBits::eHostCoherent };
        const BufferAllocation    stagingBufferAlloc = VulkanAllocator::AllocateBuffer(stagingSpec);

        // Fill out staging buffer
        void* dataPtr = VulkanAllocator::MapMemory(stagingBufferAlloc.Allocation);
        std::memcpy(dataPtr, m_Mesh->Vertices.data(), m_Mesh->GetVerticeSize());
        VulkanAllocator::UnmapMemory(stagingBufferAlloc.Allocation);

        // Transfer data from CPU to GPU
        m_Context->CopyBuffer(stagingBufferAlloc.Buffer, m_VertexBufferAlloc.Buffer, m_Mesh->GetVerticeSize());

        // Destroy staging buffer
        VulkanAllocator::DestroyBuffer(stagingBufferAlloc);

        LOG_INFO("Created and uploaded vertex buffer ...");
    }

    void VulkanModel::CreateIndexBuffer()
    {
        ASSERT(!m_Mesh->Indices.empty(), "Model has no index data!");

        // Create index buffer
        const BufferSpecification iboSpec{ .Size             = m_Mesh->GetIndiceSize(),
                                           .BufferUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer
                                                               | vk::BufferUsageFlagBits::eTransferDst,
                                           .MemoryUsage      = MemoryUsage::eGPUOnly,
                                           .MemoryFlags      = vk::MemoryPropertyFlagBits::eDeviceLocal };
        m_IndexBufferAlloc = VulkanAllocator::AllocateBuffer(iboSpec);

        // Create staging buffer
        const BufferSpecification stagingSpec{ .Size             = m_Mesh->GetIndiceSize(),
                                               .BufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
                                               .MemoryUsage      = MemoryUsage::eCPUOnly,
                                               .MemoryFlags      = vk::MemoryPropertyFlagBits::eHostVisible
                                                                   | vk::MemoryPropertyFlagBits::eHostCoherent };
        const BufferAllocation    stagingBufferAlloc = VulkanAllocator::AllocateBuffer(stagingSpec);

        // Fill out staging buffer
        void* dataPtr = VulkanAllocator::MapMemory(stagingBufferAlloc.Allocation);
        std::memcpy(dataPtr, m_Mesh->Indices.data(), m_Mesh->GetIndiceSize());
        VulkanAllocator::UnmapMemory(stagingBufferAlloc.Allocation);

        // Transfer data from CPU to GPU
        m_Context->CopyBuffer(stagingBufferAlloc.Buffer, m_IndexBufferAlloc.Buffer, m_Mesh->GetIndiceSize());

        // Destroy staging buffer
        VulkanAllocator::DestroyBuffer(stagingBufferAlloc);

        LOG_INFO("Created and uploaded index buffer ...");
    }
}
