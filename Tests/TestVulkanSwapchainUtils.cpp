// Automatically generated based on project rules

#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include "Platform/Window.hpp"

#include "Vendor/doctest/doctest.hpp"

#include <array>
#include <limits>
#include <vector>

namespace
{
    using Engine::Graphics::VulkanSwapchainUtils;
    using Engine::Platform::FramebufferExtent;

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

    // ----- Extent conversion -----

    TEST_CASE("ToVkExtent and ToFramebufferExtent keep width and height and round-trip")
    {
        const std::array<FramebufferExtent, 4> extents = { FramebufferExtent{ .Width = 0, .Height = 0 },
                                                           FramebufferExtent{ .Width = 1920, .Height = 1080 },
                                                           FramebufferExtent{ .Width = 1, .Height = 4096 },
                                                           FramebufferExtent{ .Width  = APPLICATION_DEFINED,
                                                                              .Height = APPLICATION_DEFINED - 1 } };

        for (const FramebufferExtent& extent : extents)
        {
            CAPTURE(extent.Width);
            CAPTURE(extent.Height);

            // Width and height must not be swapped
            const vk::Extent2D vkExtent = VulkanSwapchainUtils::ToVkExtent(extent);
            CHECK(vkExtent.width == extent.Width);
            CHECK(vkExtent.height == extent.Height);

            CHECK(VulkanSwapchainUtils::ToFramebufferExtent(vkExtent) == extent);
        }
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

        // Lies inside the range, so returning it instead of the surface extent would be visible
        constexpr FramebufferExtent FRAMEBUFFER{ .Width = 1280, .Height = 1024 };

        for (const vk::Extent2D& current : dictated)
        {
            CAPTURE(current.width);
            CAPTURE(current.height);

            const vk::Extent2D extent = VulkanSwapchainUtils::ChooseExtent(
                MakeCapabilities(current, { .width = 1000, .height = 1000 }, { .width = 1920, .height = 1080 }),
                FRAMEBUFFER);

            CHECK(extent.width == current.width);
            CHECK(extent.height == current.height);
        }
    }

    TEST_CASE("ChooseExtent clamps the framebuffer extent into the allowed range")
    {
        struct ClampCase
        {
            const char*       Name;
            FramebufferExtent Framebuffer;
            vk::Extent2D      Minimum;
            vk::Extent2D      Maximum;
            vk::Extent2D      Expected;
        };

        const std::array<ClampCase, 7> cases = { ClampCase{ .Name        = "inside the range",
                                                            .Framebuffer = { .Width = 1600, .Height = 900 },
                                                            .Minimum     = { .width = 1, .height = 1 },
                                                            .Maximum     = { .width = 3840, .height = 2160 },
                                                            .Expected    = { .width = 1600, .height = 900 } },
                                                 ClampCase{ .Name        = "one allowed size",
                                                            .Framebuffer = { .Width = 1920, .Height = 1080 },
                                                            .Minimum     = { .width = 1280, .height = 720 },
                                                            .Maximum     = { .width = 1280, .height = 720 },
                                                            .Expected    = { .width = 1280, .height = 720 } },
                                                 ClampCase{ .Name        = "below the minimum",
                                                            .Framebuffer = { .Width = 640, .Height = 480 },
                                                            .Minimum     = { .width = 800, .height = 600 },
                                                            .Maximum     = { .width = 3840, .height = 2160 },
                                                            .Expected    = { .width = 800, .height = 600 } },
                                                 ClampCase{ .Name        = "above the maximum",
                                                            .Framebuffer = { .Width = 4096, .Height = 4096 },
                                                            .Minimum     = { .width = 1, .height = 1 },
                                                            .Maximum     = { .width = 1920, .height = 1080 },
                                                            .Expected    = { .width = 1920, .height = 1080 } },
                                                 ClampCase{ .Name        = "axes clamped independently",
                                                            .Framebuffer = { .Width = 100, .Height = 5000 },
                                                            .Minimum     = { .width = 200, .height = 200 },
                                                            .Maximum     = { .width = 3000, .height = 3000 },
                                                            .Expected    = { .width = 200, .height = 3000 } },
                                                 ClampCase{ .Name        = "empty framebuffer raised to the minimum",
                                                            .Framebuffer = { .Width = 0, .Height = 0 },
                                                            .Minimum     = { .width = 1, .height = 1 },
                                                            .Maximum     = { .width = 3840, .height = 2160 },
                                                            .Expected    = { .width = 1, .height = 1 } },
                                                 ClampCase{ .Name        = "permissive range",
                                                            .Framebuffer = { .Width = 7680, .Height = 4320 },
                                                            .Minimum     = { .width = 0, .height = 0 },
                                                            .Maximum     = { .width  = APPLICATION_DEFINED - 1,
                                                                             .height = APPLICATION_DEFINED - 1 },
                                                            .Expected    = { .width = 7680, .height = 4320 } } };

        for (const ClampCase& testCase : cases)
        {
            INFO(testCase.Name);

            const vk::Extent2D extent = VulkanSwapchainUtils::ChooseExtent(
                MakeCapabilities({ .width = APPLICATION_DEFINED, .height = APPLICATION_DEFINED },
                                 testCase.Minimum,
                                 testCase.Maximum),
                testCase.Framebuffer);

            CHECK(extent.width == testCase.Expected.width);
            CHECK(extent.height == testCase.Expected.height);
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
