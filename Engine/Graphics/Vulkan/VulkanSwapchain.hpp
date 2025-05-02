#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine
{
    class VulkanSwapchain
    {
        public:
            VulkanSwapchain() = default;

            void Init(const VulkanDevice* device, const vk::SurfaceKHR* surface);
            void Recreate();
            void Destroy();

            [[nodiscard]] const vk::SwapchainKHR& GetHandle() const { return m_Swapchain; };

       private:
            void Create();
            void PrintDetails();

            const VulkanDevice*   m_Device  = nullptr;
            const vk::SurfaceKHR* m_Surface = nullptr;

            vk::SwapchainKHR     m_Swapchain;
            vk::Extent2D         m_Extent;
            vk::SurfaceFormatKHR m_SurfaceFormat;
            vk::PresentModeKHR   m_PresentMode;
    };
}
