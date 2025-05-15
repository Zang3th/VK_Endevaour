#include "VulkanSwapchain.hpp"
#include "VulkanAssert.hpp"

#include "Core/Window.hpp"

namespace
{
    // ----- Internal -----

    vk::Extent2D ChooseExtent(vk::SurfaceCapabilitiesKHR capabilities)
    {
        // Swapchain dimensions are fixed by the surface. Don't change anything
        if(capabilities.currentExtent.width != std::numeric_limits<u32>::max())
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

    VulkanSwapchain::VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface)
        : m_Device(device), m_Surface(surface)
    {
        FetchCapabilities();
        CreateCommandPools();
        InitializeFrames();
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        Destroy();

        // Destroy sync objects
        for(size_t i = 0; i <FRAMES_IN_FLIGHT; i++)
        {
            m_Device->GetHandle().destroySemaphore(m_Frames.at(i).ImageAvailable);
            m_Device->GetHandle().destroySemaphore(m_Frames.at(i).RenderFinished);
            m_Device->GetHandle().destroyFence(m_Frames.at(i).InFlight);
        }

        // Destroy command pools
        m_Device->GetHandle().destroyCommandPool(m_GraphicsCommandPool);
        m_Device->GetHandle().destroyCommandPool(m_TransferCommandPool);
    }

    void VulkanSwapchain::Create()
    {
        vk::SwapchainCreateInfoKHR swapchainCreate
        {
            .surface          = m_Surface,
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

    vk::CommandBuffer VulkanSwapchain::CreateTransferCommandBuffer()
    {
        // Allocate command buffer
        vk::CommandBufferAllocateInfo allocateInfo
        {
            .commandPool = m_TransferCommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto [res, commandBuffer] = m_Device->GetHandle().allocateCommandBuffers(allocateInfo);
        VK_VERIFY(res);
        ASSERT(!commandBuffer.empty(), "Allocated command buffer vector was empty!");

        // Begin command buffer recording
        vk::CommandBufferBeginInfo beginInfo
        {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        VK_VERIFY(commandBuffer.at(0).begin(&beginInfo));

        return commandBuffer.at(0);
    }

    void VulkanSwapchain::SubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer)
    {
        // End command buffer recording
        VK_VERIFY(commandBuffer.end());

        // Immediately submit recorded commands
        vk::SubmitInfo submitInfo
        {
            .commandBufferCount = 1,
            .pCommandBuffers    = &commandBuffer
        };
        vk::Queue transferQueue = m_Device->GetTransferQueue();
        VK_VERIFY(transferQueue.submit(1, &submitInfo, nullptr));
        VK_VERIFY(transferQueue.waitIdle());

        // Clean up command buffer
        m_Device->GetHandle().freeCommandBuffers(m_TransferCommandPool, 1, &commandBuffer);
    }

    [[nodiscard]] std::pair<b8, VulkanFrame&> VulkanSwapchain::GetCurrentFrame()
    {
        b8 renderNextFrame = true;

        // Get current frame
        VulkanFrame& currentFrame = m_Frames.at(m_CurrentFrame);

        // Get next image
        vk::Result res = m_Device->GetHandle().acquireNextImageKHR(m_Swapchain, UINT64_MAX, currentFrame.ImageAvailable, nullptr, &currentFrame.ImageIndex);

        // Check for resize
        if(res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR)
        {
            Recreate();
            LOG_WARN("vkAcquireNextImageKHR recreated swap chain ...");
            renderNextFrame = false;
        }
        ASSERT(res == vk::Result::eSuccess, "Failed to acquire swapchain image!");

        // Reset fence (only if we are submitting work)
        VK_VERIFY(m_Device->GetHandle().resetFences(1, &currentFrame.InFlight));

        // Reset command buffer
        currentFrame.CommandBuffer.reset();

        // Return frame
        return { renderNextFrame, currentFrame };
    }

    void VulkanSwapchain::SubmitFrame(const VulkanFrame& frame)
    {
        // TODO: Submit frame to queue

        // Advance frame counter
        m_CurrentFrame = (m_CurrentFrame + 1) % FRAMES_IN_FLIGHT;
    }

    // ----- Private -----

    void VulkanSwapchain::Recreate()
    {
        // Wait for GPU
        m_Device->WaitForIdle();

        Destroy();
        FetchCapabilities();
        Create();
    }

    void VulkanSwapchain::Destroy()
    {
        // Destroy images and image views
        for(auto& image : m_Images)
        {
            m_Device->GetHandle().destroyImage(image.Image);
            m_Device->GetHandle().destroyImageView(image.View);
        }
        m_Images.clear();

        // Destroy swap chain
        m_Device->GetHandle().destroySwapchainKHR(m_Swapchain);
    }

    void VulkanSwapchain::FetchCapabilities()
    {
        // Get swapchain capabilites from the physical device
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

    void VulkanSwapchain::CreateCommandPools()
    {
        // Create graphics pool
        {
            vk::CommandPoolCreateInfo graphicsPoolInfo
            {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = m_Device->GetGraphicsQueueFamily()
            };
            auto [res, pool] = m_Device->GetHandle().createCommandPool(graphicsPoolInfo);
            VK_VERIFY(res);
            m_GraphicsCommandPool = pool;
        }

        // Create transfer pool
        {
            vk::CommandPoolCreateInfo transferPoolInfo
            {
                .flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = m_Device->GetTransferQueueFamily()
            };
            auto [res, pool] = m_Device->GetHandle().createCommandPool(transferPoolInfo);
            VK_VERIFY(res);
            m_TransferCommandPool = pool;
        }

        LOG_INFO("Created command pools for graphics and transfer ...");
    }

    void VulkanSwapchain::InitializeFrames()
    {
        // Define semaphores
        vk::SemaphoreCreateInfo semaphoreInfo{};

        // Define fence (in signaled state to avoid endless waiting for the first frame)
        vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

        // Create sync objects
        for(u32 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VK_VERIFY(m_Device->GetHandle().createSemaphore(&semaphoreInfo, nullptr, &m_Frames[i].ImageAvailable));
            VK_VERIFY(m_Device->GetHandle().createSemaphore(&semaphoreInfo, nullptr, &m_Frames[i].RenderFinished));
            VK_VERIFY(m_Device->GetHandle().createFence(&fenceInfo, nullptr, &m_Frames[i].InFlight));
        }

        // Allocate command buffers
        vk::CommandBufferAllocateInfo allocateInfo
        {
            .commandPool        = m_GraphicsCommandPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = FRAMES_IN_FLIGHT
        };
        auto [res, commandBuffers] = m_Device->GetHandle().allocateCommandBuffers(allocateInfo);
        VK_VERIFY(res);
        ASSERT(!commandBuffers.empty(), "Allocated command buffer vector was empty!");

        // Copy command buffers in frame struct
        for(u32 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_Frames[i].CommandBuffer = commandBuffers.at(i);
        }

        LOG_INFO("Initialized sync objects and command buffers for {} frames ...", FRAMES_IN_FLIGHT);
    }
}
