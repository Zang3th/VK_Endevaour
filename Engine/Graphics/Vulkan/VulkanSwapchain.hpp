#pragma once

#include "VulkanDevice.hpp"

namespace Engine
{
    struct SwapchainImage
    {
        vk::Image     Image = nullptr;
        vk::ImageView View  = nullptr;
    };

    class VulkanSwapchain
    {
        public:
            VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface);

            void Create();
            void Recreate();
            void Destroy();

            [[nodiscard]] const vk::SwapchainKHR&            GetHandle() const { return m_Swapchain; };
            [[nodiscard]] const std::vector<SwapchainImage>& GetImages() const { return m_Images;    };

       private:
            void Init();
            void CreateImages();

            const VulkanDevice*   m_Device  = nullptr;
            const vk::SurfaceKHR& m_Surface = nullptr;

            vk::SwapchainKHR                m_Swapchain     = nullptr;
            vk::Extent2D                    m_Extent        = { .width = 0, .height = 0 };
            vk::SurfaceFormatKHR            m_SurfaceFormat = { .format = vk::Format::eUndefined,
                                                                .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };
            vk::PresentModeKHR              m_PresentMode   = vk::PresentModeKHR::eFifo;
            u32                             m_ImageCount    = 0;
            vk::SurfaceTransformFlagBitsKHR m_Transform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;
            std::vector<SwapchainImage>     m_Images;
    };
}
