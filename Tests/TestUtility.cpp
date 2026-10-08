// Automatically generated based on project rules

#include "Core/Utility.hpp"

#include "Vendor/doctest/doctest.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace
{
    using Engine::Core::Utility;

    // ----- Helpers -----

    class TempBinaryFile
    {
    public:
        TempBinaryFile(const std::vector<char>& content, const std::string& name)
            : m_Path(std::filesystem::temp_directory_path() / name)
        {
            std::ofstream file(m_Path, std::ios::binary | std::ios::trunc);
            REQUIRE(file.is_open());
            file.write(content.data(), (std::streamsize)content.size());
            file.close();

            REQUIRE(std::filesystem::exists(m_Path));
        }

        ~TempBinaryFile()
        {
            std::error_code errorCode;
            std::filesystem::remove(m_Path, errorCode);
        }

        TempBinaryFile(const TempBinaryFile&)            = delete;
        TempBinaryFile& operator=(const TempBinaryFile&) = delete;

        [[nodiscard]] const std::filesystem::path& Path() const { return m_Path; }

    private:
        std::filesystem::path m_Path;
    };

    constexpr Engine::u64 KB = 1024ull;
    constexpr Engine::u64 MB = 1024ull * KB;
    constexpr Engine::u64 GB = 1024ull * MB;

    // ----- ReadFileAsBytes -----

    TEST_CASE("ReadFileAsBytes returns the file content byte for byte")
    {
        struct FileCase
        {
            const char*       Name;
            const char*       FileName;
            std::vector<char> Content;
        };

        // Large payload covering every byte value, so no value gets translated or dropped
        std::vector<char> large(MB + 7);
        for (Engine::u64 i = 0; i < large.size(); i++)
        {
            large[i] = (char)(i % 256);
        }

        const std::array<FileCase, 4> cases = {
            FileCase{ .Name = "empty file", .FileName = "vk_utility_empty.bin", .Content = {} },
            FileCase{ .Name = "single byte", .FileName = "vk_utility_single.bin", .Content = { 'x' } },
            FileCase{ .Name     = "line endings and zero bytes",
                      .FileName = "vk_utility_text.bin",
                      .Content  = { 'a', '\r', '\n', '\0', 'b', '\n', '\x1A', '\r' } },
            FileCase{ .Name = "every byte value", .FileName = "vk_utility_large.bin", .Content = large }
        };

        for (const FileCase& testCase : cases)
        {
            INFO(testCase.Name);

            const TempBinaryFile    file(testCase.Content, testCase.FileName);
            const std::vector<char> bytes = Utility::ReadFileAsBytes(file.Path());

            REQUIRE(bytes.size() == testCase.Content.size());
            CHECK(bytes == testCase.Content);
        }
    }

    // ----- BytesToString -----

    TEST_CASE("BytesToString picks the largest unit that fits and prints two decimals")
    {
        struct BytesCase
        {
            Engine::u64 Bytes;
            const char* Expected;
        };

        const std::array<BytesCase, 10> cases = { BytesCase{ .Bytes = 0, .Expected = "0 bytes" },
                                                  BytesCase{ .Bytes = 1, .Expected = "1 bytes" },
                                                  BytesCase{ .Bytes = KB - 1, .Expected = "1023 bytes" },
                                                  BytesCase{ .Bytes = KB, .Expected = "1.00 KB" },
                                                  BytesCase{ .Bytes = KB + (KB / 2), .Expected = "1.50 KB" },
                                                  BytesCase{ .Bytes = MB, .Expected = "1.00 MB" },
                                                  BytesCase{ .Bytes = (3 * MB) + (MB / 4), .Expected = "3.25 MB" },
                                                  BytesCase{ .Bytes = GB, .Expected = "1.00 GB" },
                                                  BytesCase{ .Bytes = 5 * 1024ull * GB, .Expected = "5120.00 GB" },
                                                  BytesCase{ .Bytes    = std::numeric_limits<Engine::u64>::max(),
                                                             .Expected = "17179869184.00 GB" } };

        for (const BytesCase& testCase : cases)
        {
            CAPTURE(testCase.Bytes);
            CHECK(Utility::BytesToString(testCase.Bytes) == testCase.Expected);
        }
    }

    // ----- MillisecondsToString -----

    TEST_CASE("MillisecondsToString switches to seconds and minutes at their thresholds")
    {
        struct DurationCase
        {
            Engine::f64 Milliseconds;
            const char* Expected;
        };

        const std::array<DurationCase, 9> cases = { DurationCase{ .Milliseconds = 0.0, .Expected = "0.00 ms" },
                                                    DurationCase{ .Milliseconds = 0.004, .Expected = "0.00 ms" },
                                                    DurationCase{ .Milliseconds = 12.5, .Expected = "12.50 ms" },
                                                    DurationCase{ .Milliseconds = 999.5, .Expected = "999.50 ms" },
                                                    DurationCase{ .Milliseconds = 1000.0, .Expected = "1.00 sec" },
                                                    DurationCase{ .Milliseconds = 2500.0, .Expected = "2.50 sec" },
                                                    DurationCase{ .Milliseconds = 60000.0, .Expected = "1.00 min" },
                                                    DurationCase{ .Milliseconds = 90000.0, .Expected = "1.50 min" },
                                                    DurationCase{ .Milliseconds = -5.0, .Expected = "-5.00 ms" } };

        for (const DurationCase& testCase : cases)
        {
            CAPTURE(testCase.Milliseconds);
            CHECK(Utility::MillisecondsToString(testCase.Milliseconds) == testCase.Expected);
        }
    }

    // ----- FPSToString -----

    TEST_CASE("FPSToString prints two decimals")
    {
        struct FPSCase
        {
            Engine::f64 FPS;
            const char* Expected;
        };

        const std::array<FPSCase, 4> cases = { FPSCase{ .FPS = 0.0, .Expected = "0.00 FPS" },
                                               FPSCase{ .FPS = 60.0, .Expected = "60.00 FPS" },
                                               FPSCase{ .FPS = 143.75, .Expected = "143.75 FPS" },
                                               FPSCase{ .FPS = 10000.0, .Expected = "10000.00 FPS" } };

        for (const FPSCase& testCase : cases)
        {
            CAPTURE(testCase.FPS);
            CHECK(Utility::FPSToString(testCase.FPS) == testCase.Expected);
        }
    }

    // ----- GetRandomVec3 -----

    TEST_CASE("GetRandomVec3 stays in the unit range and varies between calls")
    {
        constexpr Engine::u32 sampleCount = 1000;

        Engine::b8                      allInRange = true;
        std::unordered_set<Engine::f32> distinct;

        for (Engine::u32 i = 0; i < sampleCount; i++)
        {
            const glm::vec3 value = Utility::GetRandomVec3();

            // Closed upper bound: float uniform_real_distribution may round up to 1.0
            for (Engine::u32 component = 0; component < 3; component++)
            {
                allInRange =
                    allInRange && value[(glm::length_t)component] >= 0.0f && value[(glm::length_t)component] <= 1.0f;
            }

            distinct.insert(value.x);
        }

        CHECK(allInRange);

        // Quality bound, not an exact count: repeated values are legal
        CHECK(distinct.size() >= (sampleCount * 95u) / 100u);
    }
}
