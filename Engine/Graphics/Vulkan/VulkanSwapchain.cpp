#include "VulkanSwapchain.hpp"
#include "VulkanAssert.hpp"
#include "Core/Window.hpp"

namespace
{
    // ----- Internal -----

    vk::Extent2D ChooseExtent(vk::SurfaceCapabilitiesKHR capabilities)
    {
        // Swapchain dimensions are fixed by the surface. Don't change anything
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            Engine::Window::SetFramebufferWidth(capabilities.currentExtent.width);
            Engine::Window::SetFramebufferHeight(capabilities.currentExtent.height);
            return capabilities.currentExtent;
        }

        Engine::Window::UpdateFramebufferSize();
        vk::Extent2D extent =
        {
            .width  = Engine::Window::GetFramebufferWidth(),
            .height = Engine::Window::GetFramebufferHeight()
        };

        // Clamp width and height between allowed min and max values of the display manager implementation
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }

    vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& surfaceFormats)
    {
        // Check if prefered format is available
        for(const auto& surfaceFormat : surfaceFormats)
        {
            if(surfaceFormat.format == vk::Format::eB8G8R8A8Srgb &&
               surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return surfaceFormat;
            }
        }

        // If that's not available return the first specified format
        return surfaceFormats[0];
    }

    vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
    {
        // Check if mailbox mode aka triple buffering is available
        for(const auto& presentMode : presentModes)
        {
            if(presentMode == vk::PresentModeKHR::eMailbox)
            {
                return presentMode;
            }
        }

        // Queue mode is guaranteed to be available
        return vk::PresentModeKHR::eFifo;
    }
}

namespace Engine
{
    // ----- Public -----

    void VulkanSwapchain::Init(const VulkanDevice* device, const vk::SurfaceKHR* surface)
    {
        m_Device  = device;
        m_Surface = surface;

        Create();
    }

    void VulkanSwapchain::Recreate()
    {

    }

    void VulkanSwapchain::Destroy()
    {
        m_Device->GetHandle().destroySwapchainKHR(m_Swapchain);
    }

    // ----- Private -----

    void VulkanSwapchain::Create()
    {
        // Choose most optimal swapchain properties
        const SwapchainSupport swapchainSupport = m_Device->GetPhysicalDevice()->GetSwapchainSupport();
        m_Extent        = ChooseExtent(swapchainSupport.Capabilities);
        m_SurfaceFormat = ChooseSurfaceFormat(swapchainSupport.Formats);
        m_PresentMode   = ChoosePresentMode(swapchainSupport.PresentModes);

        // Specify amount of images in swap chain
        m_ImageCount = swapchainSupport.Capabilities.minImageCount + 1;

        // Make sure to not exceed bounds (0 := means no limit)
        if(swapchainSupport.Capabilities.maxImageCount > 0 &&
           m_ImageCount > swapchainSupport.Capabilities.maxImageCount)
        {
            m_ImageCount = swapchainSupport.Capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR swapchainCreate
        {
            .surface          = *m_Surface,
            .minImageCount    = m_ImageCount,
            .imageFormat      = m_SurfaceFormat.format,
            .imageColorSpace  = m_SurfaceFormat.colorSpace,
            .imageExtent      = m_Extent,
            .imageArrayLayers = 1,
            .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform     = swapchainSupport.Capabilities.currentTransform,
            .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode      = m_PresentMode,
            .clipped          = vk::True,
            .oldSwapchain     = nullptr
        };

        VK_VERIFY(m_Device->GetHandle().createSwapchainKHR(&swapchainCreate, nullptr, &m_Swapchain));

        LOG_INFO("Created swapchain ... (Size: {}x{}, Format: {}, Color: {}, Mode: {})",
                 m_Extent.width, m_Extent.height, vk::to_string(m_SurfaceFormat.format),
                 vk::to_string(m_SurfaceFormat.colorSpace), vk::to_string(m_PresentMode));

        // Retrieve image handles
        auto [result, images] = m_Device->GetHandle().getSwapchainImagesKHR(m_Swapchain);
        VK_VERIFY(result);
        m_Images = std::move(images);
    }
}
