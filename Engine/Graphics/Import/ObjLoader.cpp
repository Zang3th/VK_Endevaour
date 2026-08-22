#include "ObjLoader.hpp"

#include "Core/Utility.hpp"

#include "Debug/Log.hpp"
#include "Debug/LogTable.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Vendor/tinyobjloader/tiny_obj_loader.hpp"

namespace Engine::Graphics
{
    Mesh ObjLoader::LoadMeshFromFile(const std::filesystem::path& path, ImportMode mode)
    {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn;
        std::string                      error;
        const std::string                objPath = path.string();

        // Load obj file
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, objPath.c_str(), nullptr, true, false))
        {
            if (!warn.empty())
            {
                LOG_WARN("tinyobjloader: {}", warn);
            }

            if (!error.empty())
            {
                LOG_ERROR("tinyobjloader: {}", error);
            }

            ASSERT(false, "Failed to load model '{}'", path.string());
        }

        LOG_INFO("Loaded .obj model '{}' ...", path.string());
        LOG_TABLE_BEGIN(6);
        LOG_TABLE_COLUMN("Shapes", "{}", shapes.size());
        LOG_TABLE_COLUMN("Materials", "{}", materials.size());
        LOG_TABLE_COLUMN("Positions", "{} floats", attrib.vertices.size());
        LOG_TABLE_COLUMN("Colors", "{} floats", attrib.colors.size());
        LOG_TABLE_COLUMN("Normals", "{} floats", attrib.normals.size());
        LOG_TABLE_COLUMN("TexCoords", "{} floats", attrib.texcoords.size());
        LOG_TABLE_END();

        // Hash map to store and reuse vertices (needs a hashing function and an overloaded comparison operator)
        std::unordered_map<Vertex, u32> uniqueVertices{};
        if (mode == ImportMode::OPTIMIZED)
        {
            uniqueVertices.reserve(attrib.vertices.size());
        }

        Mesh     mesh;
        const b8 hasColors    = !attrib.colors.empty();
        const b8 hasTexCoords = !attrib.texcoords.empty();

        // Combine all faces into a single mesh by iterating over all shapes
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                // Positions
                vertex.Position = { attrib.vertices.at((3 * index.vertex_index) + 0),
                                    attrib.vertices.at((3 * index.vertex_index) + 1),
                                    attrib.vertices.at((3 * index.vertex_index) + 2) };

                // Colors
                if (mode == ImportMode::TOPOLOGY_DEBUG)
                {
                    // Even without a dedicated debug mode, assigning a random color to each corner eliminates any
                    // possibility of compressing the mesh. A vertex cannot hold two different colors, so the hashing
                    // function will never merge such vertices.
                    vertex.Color = Core::Utility::GetRandomVec3();
                }
                else if (hasColors)
                {
                    vertex.Color = { attrib.colors.at((3 * index.vertex_index) + 0),
                                     attrib.colors.at((3 * index.vertex_index) + 1),
                                     attrib.colors.at((3 * index.vertex_index) + 2) };
                }
                else
                {
                    vertex.Color = { 1.0f, 1.0f, 1.0f };
                }

                // Texture coordinates
                if (hasTexCoords && index.texcoord_index != -1)
                {
                    vertex.TexCoord = {
                        attrib.texcoords.at((2 * index.texcoord_index) + 0),
                        1.0f - attrib.texcoords.at((2 * index.texcoord_index) + 1) // Flip v-axis
                    };
                }

                if (mode == ImportMode::TOPOLOGY_DEBUG)
                {
                    mesh.Indices.push_back((u32)mesh.Vertices.size());
                    mesh.Vertices.push_back(vertex);
                }
                // Only compress meshes outside of debug mode
                else
                {
                    // Try to emplace vertex in hash map
                    const auto [it, inserted] = uniqueVertices.try_emplace(vertex, (u32)mesh.Vertices.size());

                    // Success => New, unique vertex
                    if (inserted)
                    {
                        mesh.Vertices.push_back(vertex);
                    }

                    // Save index
                    mesh.Indices.push_back(it->second);
                }
            }
        }

        if (mode == ImportMode::TOPOLOGY_DEBUG)
        {
            LOG_VERBOSE("Mesh compression is deactivated in ImportMode::TOPOLOGY_DEBUG ...");
        }
        else if (mesh.Vertices.size() < mesh.Indices.size())
        {
            LOG_INFO("Compressed and reduced mesh vertices ... (Raw: {}, Unique: {})",
                     mesh.Indices.size(),
                     mesh.Vertices.size());
        }
        else
        {
            LOG_VERBOSE("Mesh was already compressed ...");
        }

        return mesh;
    }
}
