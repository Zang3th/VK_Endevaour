// Automatically generated based on project rules

#include "Graphics/Import/ObjLoader.hpp"

#include "Vendor/doctest/doctest.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace
{
    using Engine::Graphics::Color;
    using Engine::Graphics::Mesh;
    using Engine::Graphics::ObjLoader;
    using Engine::Graphics::Vertex;

    // ----- Helpers -----

    class TempObjFile
    {
    public:
        TempObjFile(const std::string& content, const std::string& name)
            : m_Path(std::filesystem::temp_directory_path() / name)
        {
            std::ofstream file(m_Path, std::ios::binary | std::ios::trunc);
            REQUIRE(file.is_open());
            file.write(content.data(), (std::streamsize)content.size());
            file.close();

            REQUIRE(std::filesystem::exists(m_Path));
        }

        ~TempObjFile()
        {
            std::error_code errorCode;
            std::filesystem::remove(m_Path, errorCode);
        }

        TempObjFile(const TempObjFile&)            = delete;
        TempObjFile& operator=(const TempObjFile&) = delete;

        [[nodiscard]] const std::filesystem::path& Path() const { return m_Path; }

    private:
        std::filesystem::path m_Path;
    };

    // Temp directory, because the working directory is the build directory
    [[nodiscard]] Mesh Load(const std::string& content, const std::string& name, Color color = Color::DEFAULT)
    {
        const TempObjFile file(content, name);

        return ObjLoader::LoadMeshFromFile(file.Path(), color);
    }

    // Accumulated, so a large mesh does not produce one assertion per index
    void CheckMeshIsWellFormed(const Mesh& mesh)
    {
        Engine::b8 allIndicesInRange = true;
        for (const Engine::u32 index : mesh.Indices)
        {
            allIndicesInRange = allIndicesInRange && (index < mesh.Vertices.size());
        }

        CHECK(allIndicesInRange);
        CHECK(mesh.Indices.size() % 3 == 0u);
        CHECK(mesh.Vertices.size() <= mesh.Indices.size());
    }

    struct ExpectedPosition
    {
        Engine::f32 X = 0.0f;
        Engine::f32 Y = 0.0f;
        Engine::f32 Z = 0.0f;
    };

    struct ExpectedTexCoord
    {
        Engine::f32 U = 0.0f;
        Engine::f32 V = 0.0f;
    };

    void CheckPosition(const Vertex& vertex, ExpectedPosition expected)
    {
        CHECK(vertex.Position.x == doctest::Approx(expected.X));
        CHECK(vertex.Position.y == doctest::Approx(expected.Y));
        CHECK(vertex.Position.z == doctest::Approx(expected.Z));
    }

    void CheckTexCoord(const Vertex& vertex, ExpectedTexCoord expected)
    {
        CHECK(vertex.TexCoord.x == doctest::Approx(expected.U));
        CHECK(vertex.TexCoord.y == doctest::Approx(expected.V));
    }

    // ----- Geometry -----

    TEST_CASE("ObjLoader loads a triangle")
    {
        const std::string positions = "v 0.0 0.0 0.0\n"
                                      "v 1.0 0.0 0.0\n"
                                      "v 0.0 1.0 0.0\n";

        Mesh mesh;

        SUBCASE("absolute face indices")
        {
            mesh = Load(positions + "f 1 2 3\n", "vke_absolute_indices.obj");
        }

        SUBCASE("negative relative face indices")
        {
            mesh = Load(positions + "f -3 -2 -1\n", "vke_negative_indices.obj");
        }

        CheckMeshIsWellFormed(mesh);
        REQUIRE(mesh.Vertices.size() == 3u);
        REQUIRE(mesh.Indices.size() == 3u);

        CheckPosition(mesh.Vertices.at(mesh.Indices.at(0)), { .X = 0.0f, .Y = 0.0f, .Z = 0.0f });
        CheckPosition(mesh.Vertices.at(mesh.Indices.at(1)), { .X = 1.0f, .Y = 0.0f, .Z = 0.0f });
        CheckPosition(mesh.Vertices.at(mesh.Indices.at(2)), { .X = 0.0f, .Y = 1.0f, .Z = 0.0f });
    }

    TEST_CASE("ObjLoader deduplicates vertices by value")
    {
        SUBCASE("shared edge keeps the corner stream intact")
        {
            const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                                   "v 1.0 0.0 0.0\n"
                                   "v 1.0 1.0 0.0\n"
                                   "v 0.0 1.0 0.0\n"
                                   "f 1 2 3\n"
                                   "f 1 3 4\n",
                                   "vke_shared_edge.obj");

            CheckMeshIsWellFormed(mesh);
            REQUIRE(mesh.Vertices.size() == 4u);
            REQUIRE(mesh.Indices.size() == 6u);

            const std::vector<Engine::u32> expectedIndices = { 0, 1, 2, 0, 2, 3 };
            CHECK(mesh.Indices == expectedIndices);

            const std::array<ExpectedPosition, 6> expected = { ExpectedPosition{ .X = 0.0f, .Y = 0.0f, .Z = 0.0f },
                                                               ExpectedPosition{ .X = 1.0f, .Y = 0.0f, .Z = 0.0f },
                                                               ExpectedPosition{ .X = 1.0f, .Y = 1.0f, .Z = 0.0f },
                                                               ExpectedPosition{ .X = 0.0f, .Y = 0.0f, .Z = 0.0f },
                                                               ExpectedPosition{ .X = 1.0f, .Y = 1.0f, .Z = 0.0f },
                                                               ExpectedPosition{ .X = 0.0f, .Y = 1.0f, .Z = 0.0f } };

            for (Engine::u32 i = 0; i < expected.size(); i++)
            {
                CheckPosition(mesh.Vertices.at(mesh.Indices.at(i)), expected.at(i));
            }
        }

        SUBCASE("identical positions from separate obj entries collapse")
        {
            const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                                   "v 1.0 0.0 0.0\n"
                                   "v 0.0 1.0 0.0\n"
                                   "v 0.0 0.0 0.0\n"
                                   "v 1.0 0.0 0.0\n"
                                   "v 0.0 1.0 0.0\n"
                                   "f 1 2 3\n"
                                   "f 4 5 6\n",
                                   "vke_value_dedup.obj");

            CheckMeshIsWellFormed(mesh);
            CHECK(mesh.Vertices.size() == 3u);
            CHECK(mesh.Indices.size() == 6u);
        }

        SUBCASE("disjoint triangles stay separate")
        {
            const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                                   "v 1.0 0.0 0.0\n"
                                   "v 0.0 1.0 0.0\n"
                                   "v 5.0 5.0 5.0\n"
                                   "v 6.0 5.0 5.0\n"
                                   "v 5.0 6.0 5.0\n"
                                   "f 1 2 3\n"
                                   "f 4 5 6\n",
                                   "vke_disjoint.obj");

            CheckMeshIsWellFormed(mesh);
            CHECK(mesh.Vertices.size() == 6u);
            CHECK(mesh.Indices.size() == 6u);
        }
    }

    TEST_CASE("ObjLoader triangulates a quad face")
    {
        const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0\n"
                               "v 1.0 1.0 0.0\n"
                               "v 0.0 1.0 0.0\n"
                               "f 1 2 3 4\n",
                               "vke_quad_face.obj");

        CheckMeshIsWellFormed(mesh);
        CHECK(mesh.Vertices.size() == 4u);
        CHECK(mesh.Indices.size() == 6u);
    }

    TEST_CASE("ObjLoader collapses a degenerate face to a single vertex")
    {
        const Mesh mesh = Load("v 2.0 3.0 4.0\n"
                               "f 1 1 1\n",
                               "vke_degenerate_face.obj");

        CheckMeshIsWellFormed(mesh);
        REQUIRE(mesh.Vertices.size() == 1u);

        const std::vector<Engine::u32> expectedIndices = { 0, 0, 0 };
        CHECK(mesh.Indices == expectedIndices);
        CheckPosition(mesh.Vertices.at(0), { .X = 2.0f, .Y = 3.0f, .Z = 4.0f });
    }

    TEST_CASE("ObjLoader merges multiple objects into one mesh")
    {
        const Mesh mesh = Load("o First\n"
                               "v 0.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0\n"
                               "v 0.0 1.0 0.0\n"
                               "f 1 2 3\n"
                               "o Second\n"
                               "v 5.0 5.0 5.0\n"
                               "v 6.0 5.0 5.0\n"
                               "v 5.0 6.0 5.0\n"
                               "f 4 5 6\n",
                               "vke_multiple_objects.obj");

        CheckMeshIsWellFormed(mesh);
        CHECK(mesh.Vertices.size() == 6u);
        CHECK(mesh.Indices.size() == 6u);
    }

    TEST_CASE("ObjLoader returns an empty mesh when there is no geometry")
    {
        Mesh mesh;

        SUBCASE("vertices without faces")
        {
            mesh = Load("v 0.0 0.0 0.0\n"
                        "v 1.0 0.0 0.0\n",
                        "vke_no_faces.obj");
        }

        SUBCASE("empty file")
        {
            mesh = Load("", "vke_empty.obj");
        }

        CHECK(mesh.Vertices.empty());
        CHECK(mesh.Indices.empty());
    }

    TEST_CASE("ObjLoader ignores comments, blank lines and carriage returns")
    {
        const Mesh mesh = Load("# exported by a tool that writes CRLF\r\n"
                               "\r\n"
                               "v 0.0 0.0 0.0\r\n"
                               "   \r\n"
                               "v 1.0 0.0 0.0\r\n"
                               "# a comment in the middle\r\n"
                               "v 0.0 1.0 0.0\r\n"
                               "f 1 2 3\r\n",
                               "vke_crlf_comments.obj");

        CheckMeshIsWellFormed(mesh);
        REQUIRE(mesh.Vertices.size() == 3u);
        CheckPosition(mesh.Vertices.at(mesh.Indices.at(1)), { .X = 1.0f, .Y = 0.0f, .Z = 0.0f });
    }

    // ----- Texture coordinates -----

    TEST_CASE("ObjLoader flips the v axis of texture coordinates")
    {
        const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0\n"
                               "v 0.0 1.0 0.0\n"
                               "vt 0.0 0.0\n"
                               "vt 1.0 0.25\n"
                               "vt 0.25 0.75\n"
                               "f 1/1 2/2 3/3\n",
                               "vke_texcoord_flip.obj");

        CheckMeshIsWellFormed(mesh);
        REQUIRE(mesh.Vertices.size() == 3u);

        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(0)), { .U = 0.0f, .V = 1.0f });
        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(1)), { .U = 1.0f, .V = 0.75f });
        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(2)), { .U = 0.25f, .V = 0.25f });
    }

    TEST_CASE("ObjLoader loads a file where only part of the faces carry texture coordinates")
    {
        const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0\n"
                               "v 0.0 1.0 0.0\n"
                               "v 2.0 0.0 0.0\n"
                               "vt 0.0 0.25\n"
                               "vt 1.0 0.5\n"
                               "vt 0.5 0.75\n"
                               "f 1/1 2/2 3/3\n"
                               "f 1 2 4\n",
                               "vke_partial_texcoords.obj");

        CheckMeshIsWellFormed(mesh);
        REQUIRE(mesh.Indices.size() == 6u);

        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(0)), { .U = 0.0f, .V = 0.75f });
        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(1)), { .U = 1.0f, .V = 0.5f });
        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(2)), { .U = 0.5f, .V = 0.25f });

        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(3)), { .U = 0.0f, .V = 0.0f });
        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(4)), { .U = 0.0f, .V = 0.0f });
        CheckTexCoord(mesh.Vertices.at(mesh.Indices.at(5)), { .U = 0.0f, .V = 0.0f });

        // 3 textured corners plus 3 untextured ones
        CHECK(mesh.Vertices.size() == 6u);
    }

    TEST_CASE("ObjLoader keeps a uv seam apart")
    {
        const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0\n"
                               "v 0.0 1.0 0.0\n"
                               "vt 0.0 0.0\n"
                               "vt 1.0 1.0\n"
                               "f 1/1 2/1 3/1\n"
                               "f 1/2 2/1 3/1\n",
                               "vke_uv_seam.obj");

        CheckMeshIsWellFormed(mesh);
        CHECK(mesh.Vertices.size() == 4u);
        CHECK(mesh.Indices.size() == 6u);
        CHECK(mesh.Indices.at(0) != mesh.Indices.at(3));
    }

    // ----- Colors -----

    TEST_CASE("File vertex colors win over the randomize flag")
    {
        const Mesh mesh = Load("v 0.0 0.0 0.0 1.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0 0.0 1.0 0.0\n"
                               "v 0.0 1.0 0.0 0.0 0.0 1.0\n"
                               "f 1 2 3\n",
                               "vke_colors_beat_randomize.obj",
                               Color::RANDOMIZE);

        REQUIRE(mesh.Vertices.size() == 3u);

        const Vertex& red = mesh.Vertices.at(mesh.Indices.at(0));
        CHECK(red.Color.r == doctest::Approx(1.0f));
        CHECK(red.Color.g == doctest::Approx(0.0f));
        CHECK(red.Color.b == doctest::Approx(0.0f));

        CHECK(mesh.Vertices.at(mesh.Indices.at(1)).Color.g == doctest::Approx(1.0f));
        CHECK(mesh.Vertices.at(mesh.Indices.at(2)).Color.b == doctest::Approx(1.0f));
    }

    TEST_CASE("Randomized colors stay inside the unit range")
    {
        const Mesh mesh = Load("v 0.0 0.0 0.0\n"
                               "v 1.0 0.0 0.0\n"
                               "v 0.0 1.0 0.0\n"
                               "f 1 2 3\n",
                               "vke_random_color_range.obj",
                               Color::RANDOMIZE);

        REQUIRE_FALSE(mesh.Vertices.empty());

        Engine::b8 allInRange = true;
        for (const Vertex& vertex : mesh.Vertices)
        {
            allInRange = allInRange && (vertex.Color.r >= 0.0f) && (vertex.Color.r <= 1.0f) && (vertex.Color.g >= 0.0f)
                         && (vertex.Color.g <= 1.0f) && (vertex.Color.b >= 0.0f) && (vertex.Color.b <= 1.0f);
        }

        CHECK(allInRange);
    }

    TEST_CASE("The color flag must not change mesh topology")
    {
        const std::string content = "v 0.0 0.0 0.0\n"
                                    "v 1.0 0.0 0.0\n"
                                    "v 1.0 1.0 0.0\n"
                                    "v 0.0 1.0 0.0\n"
                                    "f 1 2 3\n"
                                    "f 1 3 4\n";

        const Mesh plain      = Load(content, "vke_topology_plain.obj", Color::DEFAULT);
        const Mesh randomized = Load(content, "vke_topology_random.obj", Color::RANDOMIZE);

        CheckMeshIsWellFormed(randomized);
        REQUIRE(plain.Vertices.size() == 4u);

        CHECK(randomized.Vertices.size() == plain.Vertices.size());
        CHECK(randomized.Indices == plain.Indices);
    }

    // ----- Scale -----

    TEST_CASE("ObjLoader deduplicates a large grid down to the exact vertex count")
    {
        constexpr Engine::u32 quadsPerSide    = 32;
        constexpr Engine::u32 verticesPerSide = quadsPerSide + 1;

        std::string content;
        content.reserve(1u << 16);

        for (Engine::u32 y = 0; y < verticesPerSide; y++)
        {
            for (Engine::u32 x = 0; x < verticesPerSide; x++)
            {
                content += "v " + std::to_string(x) + " " + std::to_string(y) + " 0\n";
            }
        }

        for (Engine::u32 y = 0; y < quadsPerSide; y++)
        {
            for (Engine::u32 x = 0; x < quadsPerSide; x++)
            {
                // 1-based obj indices, two triangles per quad
                const Engine::u32 bottomLeft  = (y * verticesPerSide) + x + 1;
                const Engine::u32 bottomRight = bottomLeft + 1;
                const Engine::u32 topRight    = bottomLeft + verticesPerSide + 1;
                const Engine::u32 topLeft     = bottomLeft + verticesPerSide;

                content += "f " + std::to_string(bottomLeft) + " " + std::to_string(bottomRight) + " "
                           + std::to_string(topRight) + "\n";
                content += "f " + std::to_string(bottomLeft) + " " + std::to_string(topRight) + " "
                           + std::to_string(topLeft) + "\n";
            }
        }

        const Mesh mesh = Load(content, "vke_grid.obj");

        CheckMeshIsWellFormed(mesh);
        CHECK(mesh.Indices.size() == quadsPerSide * quadsPerSide * 2u * 3u);
        CHECK(mesh.Vertices.size() == verticesPerSide * verticesPerSide);

        std::unordered_set<Engine::u32> referenced(mesh.Indices.begin(), mesh.Indices.end());
        std::unordered_set<Vertex>      distinct(mesh.Vertices.begin(), mesh.Vertices.end());

        CHECK(referenced.size() == mesh.Vertices.size());
        CHECK(distinct.size() == mesh.Vertices.size());
    }

}
