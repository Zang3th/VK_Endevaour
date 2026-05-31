#pragma once

#include "Graphics/Resources/Mesh.hpp"

#include <filesystem>

namespace Engine::Graphics
{
    class ObjLoader
    {
    public:
        ObjLoader() = delete;

        static Mesh LoadMeshFromFile(const std::filesystem::path& path);
    };
}
