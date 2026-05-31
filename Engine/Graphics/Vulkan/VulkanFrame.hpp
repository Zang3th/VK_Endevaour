#pragma once

#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    // FIX: Das Frame-Konzept hier ist konzeptionell falsch.
    // Vulkan garantiert nie, dass 1 Frame ==  1 Image ist.
    // Ein Frame rendert in ein Image, welches dem WSI gehört.
    // Deswegen crashed dieser Code wenn ich auf Swapchains mit bspw. 4 Images treffe.
    // Die beiden Konzepte und Synchronisationsmechanismen müssen grundsätzlich entkoppelt werden.
    class VulkanFrame
    {
    public:
        void Begin(const SwapchainImage& image, vk::Extent2D extent) const;
        void End(const SwapchainImage& image) const;

        vk::CommandBuffer CommandBuffer  = nullptr;
        vk::Semaphore     ImageAvailable = nullptr;
        vk::Semaphore     RenderFinished = nullptr;
        vk::Fence         InFlight       = nullptr;
        u32               ImageIndex     = UINT32_MAX;
    };
}
