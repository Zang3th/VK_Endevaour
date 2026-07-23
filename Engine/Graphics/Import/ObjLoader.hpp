#pragma once

#include "Graphics/Resources/Mesh.hpp"

#include <filesystem>

namespace Engine::Graphics
{
    enum class Color : u8
    {
        DEFAULT   = 0u,
        RANDOMIZE = 1u
    };

    class ObjLoader
    {
    public:
        ObjLoader() = delete;

        static Mesh LoadMeshFromFile(const std::filesystem::path& path, Color color = Color::DEFAULT);
    };
}
