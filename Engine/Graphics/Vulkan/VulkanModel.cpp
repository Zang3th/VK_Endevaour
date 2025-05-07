#include "VulkanModel.hpp"

#include "Graphics/ObjLoader.hpp"

namespace Engine
{
    VulkanModel::VulkanModel(const vk::Device& device, const std::filesystem::path& path)
    {
        m_Mesh = ObjLoader::LoadMeshFromFile(path);
    }
}
