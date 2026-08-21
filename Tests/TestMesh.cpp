// Automatically generated based on project rules

#include "Graphics/Resources/Mesh.hpp"

#include "Vendor/doctest/doctest.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    using Engine::Graphics::Mesh;
    using Engine::Graphics::Vertex;

    // ----- Vertex layout -----

    constexpr vk::VertexInputBindingDescription BINDING    = Vertex::GetBindingDescription();
    constexpr auto                              ATTRIBUTES = Vertex::GetAttributeDescriptions();

    static_assert(BINDING.binding == 0 && BINDING.stride == sizeof(Vertex)
                      && BINDING.inputRate == vk::VertexInputRate::eVertex,
                  "Vertex binding description is out of sync with the Vertex struct");

    static_assert(ATTRIBUTES.size() == 3, "Vertex exposes exactly three attributes");

    static_assert(ATTRIBUTES[0].location == 0 && ATTRIBUTES[0].binding == BINDING.binding
                      && ATTRIBUTES[0].offset == offsetof(Vertex, Position)
                      && ATTRIBUTES[0].format == vk::Format::eR32G32B32Sfloat,
                  "Attribute 0 is out of sync with Vertex::Position");

    static_assert(ATTRIBUTES[1].location == 1 && ATTRIBUTES[1].binding == BINDING.binding
                      && ATTRIBUTES[1].offset == offsetof(Vertex, Color)
                      && ATTRIBUTES[1].format == vk::Format::eR32G32B32Sfloat,
                  "Attribute 1 is out of sync with Vertex::Color");

    static_assert(ATTRIBUTES[2].location == 2 && ATTRIBUTES[2].binding == BINDING.binding
                      && ATTRIBUTES[2].offset == offsetof(Vertex, TexCoord)
                      && ATTRIBUTES[2].format == vk::Format::eR32G32Sfloat,
                  "Attribute 2 is out of sync with Vertex::TexCoord");

    static_assert(ATTRIBUTES[0].offset + sizeof(glm::vec3) <= ATTRIBUTES[1].offset
                      && ATTRIBUTES[1].offset + sizeof(glm::vec3) <= ATTRIBUTES[2].offset
                      && ATTRIBUTES[2].offset + sizeof(glm::vec2) <= BINDING.stride,
                  "Vertex attributes overlap each other or run past the stride");

    // ----- Helpers -----

    [[nodiscard]] Vertex MakeVertex(Engine::f32 seed)
    {
        return { .Position = { seed, seed + 1.0f, seed + 2.0f },
                 .Color    = { seed + 3.0f, seed + 4.0f, seed + 5.0f },
                 .TexCoord = { seed + 6.0f, seed + 7.0f } };
    }

    [[nodiscard]] Engine::u64 HashOf(const Vertex& vertex)
    {
        return std::hash<Vertex>{}(vertex);
    }

    // ----- Equality -----

    TEST_CASE("Vertex equality covers every field")
    {
        struct DifferenceCase
        {
            const char* Name;
            void (*Mutate)(Vertex&);
        };

        const std::array<DifferenceCase, 5> cases = {
            DifferenceCase{ .Name = "position", .Mutate = [](Vertex& vertex) { vertex.Position.z += 1.0f; } },
            DifferenceCase{ .Name = "color", .Mutate = [](Vertex& vertex) { vertex.Color.y += 1.0f; } },
            DifferenceCase{ .Name = "texture coordinate", .Mutate = [](Vertex& vertex) { vertex.TexCoord.x += 1.0f; } },
            DifferenceCase{ .Name   = "one ulp",
                            .Mutate = [](Vertex& vertex)
                            { vertex.Position.x = std::nextafter(vertex.Position.x, 1.0e30f); } },
            DifferenceCase{ .Name   = "extreme magnitude",
                            .Mutate = [](Vertex& vertex)
                            { vertex.Position.x = std::numeric_limits<Engine::f32>::max(); } }
        };

        const Vertex base = MakeVertex(1.0f);
        CHECK(base == base);
        CHECK(base == MakeVertex(1.0f));

        for (const DifferenceCase& testCase : cases)
        {
            INFO(testCase.Name);

            Vertex other = base;
            testCase.Mutate(other);

            CHECK_FALSE(base == other);
            CHECK_FALSE(other == base);
        }
    }

    // ----- Hash -----

    TEST_CASE("Equal vertices hash equally")
    {
        Engine::b8 allConsistent = true;
        for (Engine::u32 i = 0; i < 256; i++)
        {
            const Engine::f32 seed = (Engine::f32)i * 0.75f;
            allConsistent          = allConsistent && (HashOf(MakeVertex(seed)) == HashOf(MakeVertex(seed)));
        }
        CHECK(allConsistent);

        const Vertex positiveZero{ .Position = { 0.0f, 1.0f, 2.0f },
                                   .Color    = { 0.0f, 0.0f, 0.0f },
                                   .TexCoord = { 0.0f, 0.0f } };
        const Vertex negativeZero{ .Position = { -0.0f, 1.0f, 2.0f },
                                   .Color    = { 0.0f, 0.0f, 0.0f },
                                   .TexCoord = { 0.0f, 0.0f } };

        REQUIRE(positiveZero == negativeZero);
        CHECK(HashOf(positiveZero) == HashOf(negativeZero));
    }

    TEST_CASE("A NaN coordinate makes a vertex unequal to itself")
    {
        const Vertex notANumber{ .Position = { std::numeric_limits<Engine::f32>::quiet_NaN(), 0.0f, 0.0f },
                                 .Color    = { 0.0f, 0.0f, 0.0f },
                                 .TexCoord = { 0.0f, 0.0f } };

        CHECK_FALSE(notANumber == notANumber);

        std::unordered_map<Vertex, Engine::u32> unique;
        unique[notANumber] = 0;
        unique[notANumber] = 1;

        CHECK(unique.size() == 2u);
    }

    TEST_CASE("Distinct vertices mostly hash distinctly")
    {
        constexpr Engine::u32 sampleCount = 1000;

        std::unordered_set<Engine::u64> hashes;
        std::unordered_set<Engine::u64> positions;

        for (Engine::u32 i = 0; i < sampleCount; i++)
        {
            const Engine::f32 f = (Engine::f32)i;
            const Vertex      vertex{ .Position = { f * 0.5f, f * 0.25f, f * 0.125f },
                                      .Color    = { (Engine::f32)(i % 7) * 0.125f,
                                                    (Engine::f32)(i % 11) * 0.0625f,
                                                    (Engine::f32)(i % 13) * 0.03125f },
                                      .TexCoord = { (Engine::f32)(i % 5) * 0.2f, (Engine::f32)(i % 3) * 0.25f } };

            hashes.insert(HashOf(vertex));
            positions.insert(std::hash<glm::vec3>{}(vertex.Position));
        }

        // Premise check
        REQUIRE(positions.size() == sampleCount);

        // Quality bound, not an exact count: collisions are legal
        CHECK(hashes.size() >= (sampleCount * 95u) / 100u);
    }

    // ----- Buffer sizes -----

    TEST_CASE("Mesh reports its buffer sizes in bytes")
    {
        Mesh mesh;

        SUBCASE("empty")
        {
            CHECK(mesh.GetVerticeSize() == 0u);
            CHECK(mesh.GetIndiceSize() == 0u);
        }

        SUBCASE("populated")
        {
            mesh.Vertices = { MakeVertex(0.0f), MakeVertex(1.0f), MakeVertex(2.0f) };
            mesh.Indices  = { 0, 1, 2, 2, 1, 0 };

            CHECK(mesh.GetVerticeSize() == (Engine::u32)(sizeof(Vertex) * 3));
            CHECK(mesh.GetIndiceSize() == (Engine::u32)(sizeof(Engine::u32) * 6));
        }
    }
}
