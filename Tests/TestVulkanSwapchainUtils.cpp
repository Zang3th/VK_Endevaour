// Automatically generated based on project rules

#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include "Platform/Window.hpp"

#include "Vendor/doctest/doctest.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace
{
    using Engine::Graphics::VulkanSwapchainUtils;
    using Engine::Platform::Window;

    constexpr Engine::u32 APPLICATION_DEFINED = std::numeric_limits<Engine::u32>::max();

    constexpr vk::SurfaceFormatKHR PREFERRED_FORMAT{ .format     = vk::Format::eB8G8R8A8Srgb,
                                                     .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };

    [[nodiscard]] vk::SurfaceCapabilitiesKHR MakeCapabilities(vk::Extent2D current,
                                                              vk::Extent2D minimum,
                                                              vk::Extent2D maximum)
    {
        return { .currentExtent = current, .minImageExtent = minimum, .maxImageExtent = maximum };
    }

    void CheckFormat(vk::SurfaceFormatKHR chosen, vk::SurfaceFormatKHR expected)
    {
        CHECK(chosen.format == expected.format);
        CHECK(chosen.colorSpace == expected.colorSpace);
    }

    // ----- ChooseExtent -----

    TEST_CASE("ChooseExtent passes a surface dictated extent through untouched")
    {
        // Every row lies outside the range passed below
        const std::array<vk::Extent2D, 4> dictated = { vk::Extent2D{ .width = 800, .height = 600 },
                                                       vk::Extent2D{ .width = 4096, .height = 4096 },
                                                       vk::Extent2D{ .width = 0, .height = 0 },
                                                       vk::Extent2D{ .width  = APPLICATION_DEFINED - 1,
                                                                     .height = APPLICATION_DEFINED } };

        for (const vk::Extent2D& current : dictated)
        {
            CAPTURE(current.width);
            CAPTURE(current.height);

            const vk::Extent2D extent = VulkanSwapchainUtils::ChooseExtent(
                MakeCapabilities(current, { .width = 1000, .height = 1000 }, { .width = 1920, .height = 1080 }));

            CHECK(extent.width == current.width);
            CHECK(extent.height == current.height);
        }
    }

    TEST_CASE("ChooseExtent clamps the window size into the allowed range")
    {
        struct ClampCase
        {
            const char*  Name;
            vk::Extent2D Minimum;
            vk::Extent2D Maximum;
        };

        const std::array<ClampCase, 4> cases = { ClampCase{ .Name    = "one allowed size",
                                                            .Minimum = { .width = 1280, .height = 720 },
                                                            .Maximum = { .width = 1280, .height = 720 } },
                                                 ClampCase{ .Name    = "window below the minimum",
                                                            .Minimum = { .width = 100000, .height = 100000 },
                                                            .Maximum = { .width = 200000, .height = 200000 } },
                                                 ClampCase{ .Name    = "window above the maximum",
                                                            .Minimum = { .width = 1, .height = 1 },
                                                            .Maximum = { .width = 16, .height = 16 } },
                                                 ClampCase{ .Name    = "permissive range",
                                                            .Minimum = { .width = 0, .height = 0 },
                                                            .Maximum = { .width  = APPLICATION_DEFINED - 1,
                                                                         .height = APPLICATION_DEFINED - 1 } } };

        // Expectations are derived from the window, not hardcoded, so its size does not matter
        for (const ClampCase& testCase : cases)
        {
            INFO(testCase.Name);

            const vk::Extent2D extent = VulkanSwapchainUtils::ChooseExtent(MakeCapabilities(
                { .width = APPLICATION_DEFINED, .height = APPLICATION_DEFINED }, testCase.Minimum, testCase.Maximum));

            CHECK(extent.width == std::clamp(Window::GetWidth(), testCase.Minimum.width, testCase.Maximum.width));
            CHECK(extent.height == std::clamp(Window::GetHeight(), testCase.Minimum.height, testCase.Maximum.height));
        }
    }

    // ----- ChooseSurfaceFormat -----

    TEST_CASE("ChooseSurfaceFormat requires format and color space to match and otherwise takes the first entry")
    {
        constexpr vk::SurfaceFormatKHR UNORM{ .format     = vk::Format::eR8G8B8A8Unorm,
                                              .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };
        constexpr vk::SurfaceFormatKHR HDR{ .format     = vk::Format::eB8G8R8A8Srgb,
                                            .colorSpace = vk::ColorSpaceKHR::eHdr10St2084EXT };
        constexpr vk::SurfaceFormatKHR SRGB_RGBA{ .format     = vk::Format::eR8G8B8A8Srgb,
                                                  .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };

        struct FormatCase
        {
            const char*                       Name;
            std::vector<vk::SurfaceFormatKHR> Available;
            vk::SurfaceFormatKHR              Expected;
        };

        // Decoys ahead of the match, so a fallback that never scans cannot pass
        std::vector<vk::SurfaceFormatKHR> preferredAtTheEnd(64, UNORM);
        preferredAtTheEnd.push_back(PREFERRED_FORMAT);

        const std::array<FormatCase, 5> cases = {
            FormatCase{ .Name      = "preferred at the front",
                        .Available = { PREFERRED_FORMAT, UNORM },
                        .Expected  = PREFERRED_FORMAT },
            FormatCase{ .Name = "preferred at the end", .Available = preferredAtTheEnd, .Expected = PREFERRED_FORMAT },
            FormatCase{ .Name = "right format, wrong color space", .Available = { UNORM, HDR }, .Expected = UNORM },
            FormatCase{
                .Name = "right color space, wrong format", .Available = { SRGB_RGBA, UNORM }, .Expected = SRGB_RGBA },
            FormatCase{ .Name = "single entry", .Available = { UNORM }, .Expected = UNORM }
        };

        for (const FormatCase& testCase : cases)
        {
            INFO(testCase.Name);
            CheckFormat(VulkanSwapchainUtils::ChooseSurfaceFormat(testCase.Available), testCase.Expected);
        }
    }

    // ----- ChoosePresentMode -----

    TEST_CASE("ChoosePresentMode prefers mailbox and otherwise falls back to fifo")
    {
        struct PresentCase
        {
            const char*                     Name;
            std::vector<vk::PresentModeKHR> Available;
            vk::PresentModeKHR              Expected;
        };

        const std::array<PresentCase, 5> cases = {
            PresentCase{ .Name      = "mailbox among others",
                         .Available = { vk::PresentModeKHR::eImmediate,
                                        vk::PresentModeKHR::eMailbox,
                                        vk::PresentModeKHR::eFifo },
                         .Expected  = vk::PresentModeKHR::eMailbox },
            PresentCase{ .Name      = "mailbox as the last entry",
                         .Available = { vk::PresentModeKHR::eFifo,
                                        vk::PresentModeKHR::eFifoRelaxed,
                                        vk::PresentModeKHR::eMailbox },
                         .Expected  = vk::PresentModeKHR::eMailbox },
            PresentCase{ .Name      = "mailbox missing",
                         .Available = { vk::PresentModeKHR::eImmediate, vk::PresentModeKHR::eFifoRelaxed },
                         .Expected  = vk::PresentModeKHR::eFifo },
            PresentCase{
                .Name      = "duplicates",
                .Available = { vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eMailbox },
                .Expected  = vk::PresentModeKHR::eMailbox },
            PresentCase{ .Name = "empty list", .Available = {}, .Expected = vk::PresentModeKHR::eFifo }
        };

        for (const PresentCase& testCase : cases)
        {
            INFO(testCase.Name);
            CHECK(VulkanSwapchainUtils::ChoosePresentMode(testCase.Available) == testCase.Expected);
        }
    }
}
