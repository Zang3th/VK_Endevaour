#include "VulkanModel.hpp"

#include "Graphics/ObjLoader.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanModel::VulkanModel(const vk::Device& device, const std::filesystem::path& path)
    {
        m_Mesh = ObjLoader::LoadMeshFromFile(path);
        CreateVertexBuffer();
        CreateIndexBuffer();
    }

    VulkanModel::~VulkanModel()
    {

    }

    // ----- Private -----

    void VulkanModel::CreateVertexBuffer()
    {

    }

    void VulkanModel::CreateIndexBuffer()
    {

    }
}
