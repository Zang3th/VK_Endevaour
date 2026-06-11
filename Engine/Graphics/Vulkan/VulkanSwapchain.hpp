#pragma once

#include "Graphics/Vulkan/VulkanDevice.hpp"
#include "Graphics/Vulkan/VulkanFrame.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"

#include <optional>

namespace Engine::Graphics
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface);
        ~VulkanSwapchain();

        VulkanSwapchain(const VulkanSwapchain&)            = delete;
        VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

        [[nodiscard]] const vk::SwapchainKHR&    GetHandle() const { return m_CurrentSwapchain; };
        [[nodiscard]] const SwapchainProperties& GetProperties() const { return m_Properties; };
        [[nodiscard]] const SwapchainImage&      GetImageAt(u32 index) const { return m_Images.at(index); };

        [[nodiscard]] vk::CommandBuffer CreateTransferCommandBuffer();
        void                            SubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer);

        [[nodiscard]] VulkanFrame&       GetCurrentFrame();
        [[nodiscard]] std::optional<u32> AcquireImage(const VulkanFrame& frame);

        [[nodiscard]] u32 GetImageCount() const { return m_Images.size(); }

        void ResetFrame(const VulkanFrame& frame);
        void SubmitFrame(const VulkanFrame& frame, u32 imageIndex);
        void PresentFrame(u32 imageIndex);
        void AdvanceFrameCount();

        void SetResizeFlag() { m_Resized = true; };

    private:
        void CreateSwapchain();
        void RecreateSwapchain();
        void CreateImages();
        void DestroyImages();
        void CreateCommandPools();
        void InitializeFrames();

        // Handles
        const VulkanDevice*   m_Device  = nullptr;
        const vk::SurfaceKHR& m_Surface = nullptr;

        vk::SwapchainKHR m_CurrentSwapchain = nullptr;
        vk::SwapchainKHR m_OldSwapchain     = nullptr;

        // Properties
        SwapchainProperties m_Properties;
        b8                  m_Resized = false;

        // Command pools
        vk::CommandPool m_GraphicsCommandPool;
        vk::CommandPool m_TransferCommandPool;

        // Swapchain images
        std::vector<SwapchainImage> m_Images;

        // Frames
        std::array<VulkanFrame, FRAMES_IN_FLIGHT> m_Frames;
        u32                                       m_CurrentFrame = 0;
    };
}
