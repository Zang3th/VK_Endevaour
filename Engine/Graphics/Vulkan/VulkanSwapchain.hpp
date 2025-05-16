#pragma once

#include "VulkanDevice.hpp"
#include "VulkanFrame.hpp"
#include "VulkanGlobals.hpp"

namespace Engine
{
    class VulkanSwapchain
    {
        public:
            VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface);
            ~VulkanSwapchain();

            [[nodiscard]] const vk::SwapchainKHR&    GetHandle()       const { return m_Swapchain;                 };
            [[nodiscard]] const SwapchainProperties& GetProperties()   const { return m_Properties;                };
            [[nodiscard]] const SwapchainImage&      GetCurrentImage() const { return m_Images.at(m_CurrentFrame); };

            [[nodiscard]] vk::CommandBuffer CreateTransferCommandBuffer();
            void                            SubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer);

            std::pair<b8, VulkanFrame&> GetNextFrame();
            void                        SubmitFrame(const VulkanFrame& frame);
            void                        PresentFrame(const VulkanFrame& frame);
            void                        AdvanceFrameCount();

        private:
            void Create();
            void Destroy();
            void Recreate();
            void CreateImages();
            void CreateCommandPools();
            void InitializeFrames();

            // Handles
            const VulkanDevice*    m_Device    = nullptr;
            const vk::SurfaceKHR&  m_Surface   = nullptr;
                  vk::SwapchainKHR m_Swapchain = nullptr;

            // Properties
            SwapchainProperties m_Properties;

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
