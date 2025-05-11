#include "ObjLoader.hpp"
#include "Debug/Log.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Vendor/tinyobjloader/tiny_obj_loader.hpp"

namespace Engine
{
    Mesh ObjLoader::LoadMeshFromFile(const std::filesystem::path& path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string error;

        // Load obj file
        bool success = LoadObj(&attrib, &shapes, &materials, &warn, &error, path.c_str());
        if(!success)
        {
            if(!warn.empty())  { LOG_WARN("tinyobjloader: {}", warn);   };
            if(!error.empty()) { LOG_ERROR("tinyobjloader: {}", error); };
            ASSERT(success, "Failed to load model '{}'", path.c_str());
        }
        LOG_INFO("Loaded model '{}' ... (Shapes: {}, Vertices: {}, Indices: {})", path.string(), shapes.size(), attrib.vertices.size(), shapes.at(0).mesh.indices.size());

        // Hash map to store and reuse vertices (needs a hashing function and overloaded comparison operator
        std::unordered_map<Vertex, u32> uniqueVertices{};

        Mesh mesh;

        // Combine all faces into a single mesh by iterating over all shapes
        for(const auto& shape : shapes)
        {
            for(const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.Position =
                {
                    attrib.vertices[(3 * index.vertex_index) + 0],
                    attrib.vertices[(3 * index.vertex_index) + 1],
                    attrib.vertices[(3 * index.vertex_index) + 2]
                };

                vertex.Color = {1.0f, 1.0f, 1.0f};

                vertex.TexCoord =
                {
                    attrib.texcoords[(2 * index.texcoord_index) + 0],
                    1.0f - attrib.texcoords[(2 * index.texcoord_index) + 1] // Flip v-axis
                };

                // Check for duplicate vertex
                if(!uniqueVertices.contains(vertex))
                {
                    uniqueVertices[vertex] = (u32)(mesh.Vertices.size());
                    mesh.Vertices.push_back(vertex);
                }

                // Save index
                mesh.Indices.push_back(uniqueVertices[vertex]);
            }
        }

        LOG_INFO("Compressed and reduced mesh ... (Vertices: {})", mesh.Vertices.size());

        return mesh;
    }
}
