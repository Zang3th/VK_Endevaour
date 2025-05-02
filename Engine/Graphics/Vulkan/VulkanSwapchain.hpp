#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine
{
    struct SwapchainSupport
    {
        vk::SurfaceCapabilitiesKHR Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR> PresentModes;
    };

    struct SwapchainProperties
    {
        vk::Extent2D Extent;
        vk::SurfaceFormatKHR SurfaceFormat;
        vk::PresentModeKHR PresentMode;
    };

    class VulkanSwapchain
    {
        public:
            VulkanSwapchain() = default;

            void Init(const VulkanDevice* device, const vk::SurfaceKHR* surface);
            void Create(u32 width, u32 height);
            void Recreate(u32 width, u32 height);
            void Destroy();

            [[nodiscard]] const vk::SwapchainKHR&    GetHandle()     const { return m_Swapchain;  };
            [[nodiscard]] const SwapchainSupport&    GetSupport()    const { return m_Support;    };
            [[nodiscard]] const SwapchainProperties& GetProperties() const { return m_Properties; };

       private:
            void QuerySwapchainSupport(vk::PhysicalDevice physicalDevice);

            const VulkanDevice*           m_Device         = nullptr;
            const vk::SurfaceKHR*         m_Surface        = nullptr;

            vk::SwapchainKHR    m_Swapchain;
            SwapchainSupport    m_Support;
            SwapchainProperties m_Properties;
    };
}
