#pragma once

#include "Graphics/Resources/Mesh.hpp"

#include <filesystem>

namespace Engine::Graphics
{
    enum class ImportMode : u8
    {
        eOptimized     = 0u,
        eTopologyDebug = 1u,
    };

    class ObjLoader
    {
    public:
        ObjLoader() = delete;

        static Mesh LoadMeshFromFile(const std::filesystem::path& path, ImportMode mode = ImportMode::eOptimized);
    };
}
