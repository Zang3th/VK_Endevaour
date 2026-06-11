#pragma once

#include "Graphics/Vulkan/VulkanDevice.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"
#include "Graphics/Vulkan/VulkanSwapchainStructs.hpp"

#include "Vendor/glm/vec4.hpp"

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

        [[nodiscard]] u32 GetImageCount() const { return m_Images.size(); }

        [[nodiscard]] vk::CommandBuffer CreateTransferCommandBuffer();
        void                            SubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer);

        [[nodiscard]] std::optional<SwapchainFrame> BeginFrame();

        void BeginRendering(const SwapchainFrame& frame, glm::vec4 clearColor = { 1.0f, 1.0f, 1.0f, 1.0f });
        void EndRendering(const SwapchainFrame& frame);
        void SubmitAndPresent(const SwapchainFrame& frame);

        void SetResizeFlag() { m_Resized = true; };

    private:
        void CreateSwapchain();
        void RecreateSwapchain();
        void CreateImages();
        void DestroyImages();
        void CreateCommandPools();
        void InitializeFrames();
        void AdvanceFrameCount();

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
        std::array<VulkanFrameResources, FRAMES_IN_FLIGHT> m_FrameResources;
        u32                                                m_CurrentFrame = 0;
    };
}
