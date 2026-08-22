#pragma once

#include "Graphics/Resources/Mesh.hpp"

#include <filesystem>

namespace Engine::Graphics
{
    enum class ImportMode : u8
    {
        OPTIMIZED      = 0u,
        TOPOLOGY_DEBUG = 1u
    };

    class ObjLoader
    {
    public:
        ObjLoader() = delete;

        static Mesh LoadMeshFromFile(const std::filesystem::path& path, ImportMode mode = ImportMode::OPTIMIZED);
    };
}
