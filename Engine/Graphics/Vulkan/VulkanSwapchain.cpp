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
        extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
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

    VulkanSwapchain::VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR* surface)
        : m_Device(device), m_Surface(surface)
    {
        Init();
    }

    void VulkanSwapchain::Create()
    {
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
            .preTransform     = m_Transform,
            .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode      = m_PresentMode,
            .clipped          = vk::True,
            .oldSwapchain     = nullptr
        };

        VK_VERIFY(m_Device->GetHandle().createSwapchainKHR(&swapchainCreate, nullptr, &m_Swapchain));

        LOG_INFO("Created swapchain ... (Size: {}x{}, Format: {}, Color: {}, Mode: {})",
                 m_Extent.width, m_Extent.height, vk::to_string(m_SurfaceFormat.format),
                 vk::to_string(m_SurfaceFormat.colorSpace), vk::to_string(m_PresentMode));

        CreateImages();
    }

    void VulkanSwapchain::Recreate()
    {
        Destroy();

        // Query new dimensions
        const SwapchainSupport swapchainSupport = m_Device->GetPhysicalDevice()->GetSwapchainSupport();
        m_Extent = ChooseExtent(swapchainSupport.Capabilities);

        Create();
    }

    void VulkanSwapchain::Destroy()
    {
        for(auto& image : m_Images)
        {
            m_Device->GetHandle().destroyImageView(image.View);
        }

        m_Images.clear();
        m_Device->GetHandle().destroySwapchainKHR(m_Swapchain);
    }

    // ----- Private -----

    void VulkanSwapchain::Init()
    {
        // Query swapchain capabilites from the physical device
        const SwapchainSupport swapchainSupport = m_Device->GetPhysicalDevice()->GetSwapchainSupport();

        // Choose most optimal swapchain properties
        m_Extent        = ChooseExtent(swapchainSupport.Capabilities);
        m_SurfaceFormat = ChooseSurfaceFormat(swapchainSupport.Formats);
        m_PresentMode   = ChoosePresentMode(swapchainSupport.PresentModes);

        // Specify amount of images in swapchain
        m_ImageCount = swapchainSupport.Capabilities.minImageCount + 1;

        // Make sure to not exceed bounds (0 := means no limit)
        if(swapchainSupport.Capabilities.maxImageCount > 0 &&
           m_ImageCount > swapchainSupport.Capabilities.maxImageCount)
        {
            m_ImageCount = swapchainSupport.Capabilities.maxImageCount;
        }

        // Save current transform
        m_Transform = swapchainSupport.Capabilities.currentTransform;
    }

    void VulkanSwapchain::CreateImages()
    {
        // Retrieve image handles
        auto [result, images] = m_Device->GetHandle().getSwapchainImagesKHR(m_Swapchain);
        VK_VERIFY(result);

        // Reserve space
        m_Images.reserve(images.size());

        // Create an image view for every image in the swapchain
        for(const auto& image : images)
        {
            vk::ImageViewCreateInfo viewCreateInfo =
            {
                .image    = image,
                .viewType = vk::ImageViewType::e2D,
                .format   = m_SurfaceFormat.format,
                .subresourceRange =
                {
                    .aspectMask     = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                }
            };

            auto [result, view] = m_Device->GetHandle().createImageView(viewCreateInfo);
            VK_VERIFY(result);

            m_Images.emplace_back(image, view);
        }

        LOG_INFO("Created {} image view(s) ...", images.size());
    }
}
