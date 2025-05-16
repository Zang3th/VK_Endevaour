#pragma once

#include "VulkanDevice.hpp"
#include "VulkanGlobals.hpp"

namespace Engine
{
    struct SwapchainImage
    {
        vk::Image     Image = nullptr;
        vk::ImageView View  = nullptr;
    };

    struct VulkanFrame
    {
        vk::CommandBuffer CommandBuffer;
        vk::Semaphore     ImageAvailable;
        vk::Semaphore     RenderFinished;
        vk::Fence         InFlight;
        u32               ImageIndex = UINT32_MAX;
    };

    class VulkanSwapchain
    {
        public:
            VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface);
            ~VulkanSwapchain();

            void Create();

            [[nodiscard]] const vk::SwapchainKHR&            GetHandle()        const { return m_Swapchain;     };
            [[nodiscard]] const std::vector<SwapchainImage>& GetImages()        const { return m_Images;        };
            [[nodiscard]] const vk::SurfaceFormatKHR&        GetSurfaceFormat() const { return m_SurfaceFormat; };

            [[nodiscard]] vk::CommandBuffer CreateTransferCommandBuffer();
            void                            SubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer);

            [[nodiscard]] vk::Viewport GetViewport() const {
                return { .width  = (float)m_Extent.width, .height = (float)m_Extent.height, .minDepth = 0.0f, .maxDepth = 1.0f }; };
            [[nodiscard]] vk::Rect2D GetScissor() const { return { .extent = m_Extent };      };
            [[nodiscard]] vk::Extent2D GetExtent() const { return m_Extent; };

            std::pair<b8, VulkanFrame&> GetCurrentFrame();
            void                        SubmitFrame(const VulkanFrame& frame);
            void                        PresentFrame(const VulkanFrame& frame);
            void                        AdvanceFrame();

            [[nodiscard]] const SwapchainImage& GetCurrentImage() const { return m_Images.at(m_CurrentFrame); };

        private:
            void Recreate();
            void Destroy();
            void FetchCapabilities();
            void CreateImages();
            void CreateCommandPools();
            void InitializeFrames();

            const VulkanDevice*   m_Device  = nullptr;
            const vk::SurfaceKHR& m_Surface = nullptr;

            // Internal handle
            vk::SwapchainKHR m_Swapchain = nullptr;

            // Properties
            vk::Extent2D                    m_Extent        = { .width = 0, .height = 0 };
            vk::SurfaceFormatKHR            m_SurfaceFormat = { .format = vk::Format::eUndefined,
                                                                .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };
            vk::PresentModeKHR              m_PresentMode   = vk::PresentModeKHR::eFifo;
            u32                             m_ImageCount    = 0;
            vk::SurfaceTransformFlagBitsKHR m_Transform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;

            // Command pools
            vk::CommandPool m_GraphicsCommandPool;
            vk::CommandPool m_TransferCommandPool;

            // Swapchain images
            std::vector<SwapchainImage> m_Images;

            // Frames
            std::array<VulkanFrame, FRAMES_IN_FLIGHT> m_Frames;
            u32 m_CurrentFrame = 0;
    };
}
