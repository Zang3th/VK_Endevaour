#pragma once

#include "Mesh.hpp"

#include <filesystem>

namespace Engine
{
    class ObjLoader
    {
        public:
            ObjLoader() = delete;

            static Mesh LoadMeshFromFile(const std::filesystem::path& path);
    };
}
