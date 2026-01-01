#pragma once

#include "Core/Types.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <Vendor/glm/glm.hpp>
#include <Vendor/glm/gtx/hash.hpp>

#include <vulkan/vulkan.hpp>

#include <array>
#include <vector>

namespace Engine
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec2 TexCoord;

        bool operator==(const Vertex& vertex) const
        {
            return Position == vertex.Position && Color == vertex.Color && TexCoord == vertex.TexCoord;
        }

        static constexpr vk::VertexInputBindingDescription GetBindingDescription()
        {
            return
            {
                .binding = 0,
                .stride  = sizeof(Vertex),
                .inputRate = vk::VertexInputRate::eVertex
            };
        }

        static constexpr std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
        {
            return std::to_array<vk::VertexInputAttributeDescription>
            ({
                {
                    .location = 0,
                    .binding  = 0,
                    .format   = vk::Format::eR32G32B32Sfloat,
                    .offset   = offsetof(Vertex, Position)
                },
                {
                    .location = 1,
                    .binding  = 0,
                    .format   = vk::Format::eR32G32B32Sfloat,
                    .offset   = offsetof(Vertex, Color)
                },
                {
                    .location = 2,
                    .binding  = 0,
                    .format   = vk::Format::eR32G32Sfloat,
                    .offset   = offsetof(Vertex, TexCoord)
                }
            });
        }
    };

    struct Mesh
    {
        std::vector<Vertex> Vertices;
        std::vector<u32>    Indices;

        [[nodiscard]] u32 GetVerticeSize() const { return sizeof(Vertex) * Vertices.size(); };
        [[nodiscard]] u32 GetIndiceSize()  const { return sizeof(u32)    * Indices.size();  };
    };
}

namespace std
{
    template<> struct hash<Engine::Vertex>
    {
        size_t operator()(Engine::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.Position) ^
                    (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^
                    (hash<glm::vec2>()(vertex.TexCoord) << 1);
        }
    };
}
