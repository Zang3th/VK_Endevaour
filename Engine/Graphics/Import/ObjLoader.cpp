#include "ObjLoader.hpp"

#include "Debug/Log.hpp"
#include "Debug/LogTable.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Vendor/tinyobjloader/tiny_obj_loader.hpp"

namespace Engine::Graphics
{
    Mesh ObjLoader::LoadMeshFromFile(const std::filesystem::path& path)
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

        Mesh mesh;
        b8   gotCompressed = false;

        // Combine all faces into a single mesh by iterating over all shapes
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.Position = { attrib.vertices[(3 * index.vertex_index) + 0],
                                    attrib.vertices[(3 * index.vertex_index) + 1],
                                    attrib.vertices[(3 * index.vertex_index) + 2] };

                if (attrib.colors.size() > 0)
                {
                    vertex.Color = { attrib.colors[(3 * index.vertex_index) + 0],
                                     attrib.colors[(3 * index.vertex_index) + 1],
                                     attrib.colors[(3 * index.vertex_index) + 2] };
                }

                if (attrib.texcoords.size() > 0)
                {
                    vertex.TexCoord = {
                        attrib.texcoords[(2 * index.texcoord_index) + 0],
                        1.0f - attrib.texcoords[(2 * index.texcoord_index) + 1] // Flip v-axis
                    };
                }

                // Check for duplicate vertex
                if (!uniqueVertices.contains(vertex))
                {
                    uniqueVertices[vertex] = (u32)(mesh.Vertices.size());
                    mesh.Vertices.push_back(vertex);
                }
                else
                {
                    gotCompressed = true;
                }

                // Save index
                mesh.Indices.push_back(uniqueVertices[vertex]);
            }
        }

        if (gotCompressed)
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
