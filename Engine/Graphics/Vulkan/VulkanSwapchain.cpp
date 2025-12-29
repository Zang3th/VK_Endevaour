#include "VulkanSwapchain.hpp"
#include "VulkanAssert.hpp"

#include "Core/Window.hpp"

namespace
{
    // ----- Internal -----

    Engine::SwapchainProperties GetSwapchainProperties(const Engine::VulkanPhysicalDevice* physicalDevice)
    {
        Engine::SwapchainProperties properties{};

        // Get swapchain capabilites from the physical device
        const Engine::SwapchainSupport swapchainSupport = physicalDevice->GetSwapchainSupport();

        // Choose most optimal swapchain properties
        properties.Extent        = Engine::VulkanSwapchainUtils::ChooseExtent(swapchainSupport.Capabilities);
        properties.SurfaceFormat = Engine::VulkanSwapchainUtils::ChooseSurfaceFormat(swapchainSupport.Formats);
        properties.PresentMode   = Engine::VulkanSwapchainUtils::ChoosePresentMode(swapchainSupport.PresentModes);

        // Specify amount of images in swapchain
        properties.ImageCount = swapchainSupport.Capabilities.minImageCount + 1;

        // Make sure to not exceed bounds (0 := means no limit)
        if(swapchainSupport.Capabilities.maxImageCount > 0 &&
           properties.ImageCount > swapchainSupport.Capabilities.maxImageCount)
        {
            properties.ImageCount = swapchainSupport.Capabilities.maxImageCount;
        }

        // Save current transform
        properties.Transform = swapchainSupport.Capabilities.currentTransform;

        return properties;
    }

    void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto* swapchain = (Engine::VulkanSwapchain*)glfwGetWindowUserPointer(Engine::Window::GetHandle());
        if(swapchain->GetProperties().Extent.width  != (u32)width ||
           swapchain->GetProperties().Extent.height != (u32)height)
        {
            LOG_WARN("GLFW: FramebufferResizeCallback ... (Size: {}x{})", width, height);
            Engine::Window::SetWidth((u32)width);
            Engine::Window::SetHeight((u32)height);
            swapchain->SetResizeFlag();
        }
    }
}

namespace Engine
{
    // ----- Public -----

    VulkanSwapchain::VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface)
        : m_Device(device), m_Surface(surface), m_Resized(false)
    {
        m_Properties = GetSwapchainProperties(device->GetPhysicalDevice());
        CreateCommandPools();
        InitializeFrames();
        CreateSwapchain();
        CreateImages();

        // Set framebuffer resize callback
        glfwSetWindowUserPointer(Window::GetHandle(), this);
        glfwSetFramebufferSizeCallback(Window::GetHandle(), FramebufferResizeCallback);
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        LOG_INFO("VulkanSwapchain::Destructor() ...");

        DestroyImages();

        // Destroy sync objects
        for(size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_Device->GetHandle().destroySemaphore(m_Frames.at(i).ImageAvailable);
            m_Device->GetHandle().destroySemaphore(m_Frames.at(i).RenderFinished);
            m_Device->GetHandle().destroyFence(m_Frames.at(i).InFlight);
        }

        // Destroy command pools
        m_Device->GetHandle().destroyCommandPool(m_GraphicsCommandPool);
        m_Device->GetHandle().destroyCommandPool(m_TransferCommandPool);

        // Destroy swapchain
        m_Device->GetHandle().destroySwapchainKHR(m_CurrentSwapchain);
    }

    vk::CommandBuffer VulkanSwapchain::CreateTransferCommandBuffer()
    {
        // Allocate command buffer
        vk::CommandBufferAllocateInfo allocateInfo
        {
            .commandPool        = m_TransferCommandPool,
            .level              = vk::CommandBufferLevel::ePrimary,
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

    [[nodiscard]] std::pair<b8, VulkanFrame&> VulkanSwapchain::GetNextFrame()
    {
        // Get current frame
        VulkanFrame& currentFrame = m_Frames.at(m_CurrentFrame);

        //  Wait for this frame-slot's previous submission to finish
        VK_VERIFY(m_Device->GetHandle().waitForFences(1, &currentFrame.InFlight, vk::True, UINT64_MAX));

        // TODO: This needs major rework. I should only work with the swapchain image index
        // Aquire
        vk::Result res = m_Device->GetHandle().acquireNextImageKHR(m_CurrentSwapchain, UINT64_MAX, currentFrame.ImageAvailable, nullptr, &currentFrame.ImageIndex);
        if(res == vk::Result::eErrorOutOfDateKHR)
        {
            LOG_WARN("vkAcquireNextImageKHR initialized swapchain recreation ...");
            RecreateSwapchain();
            return { false, currentFrame };
        }
        ASSERT(res == vk::Result::eSuccess, "Failed to acquire swapchain image!");

        // Reset fence
        VK_VERIFY(m_Device->GetHandle().resetFences(1, &currentFrame.InFlight));

        // Reset command buffer
        VK_VERIFY(currentFrame.CommandBuffer.reset());

        // Return frame
        return { true, currentFrame };
    }

    void VulkanSwapchain::SubmitFrame(const VulkanFrame& frame)
    {
        vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        // Create submit info
        vk::SubmitInfo submitInfo
        {
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &frame.ImageAvailable,  // On which semaphore to wait
            .pWaitDstStageMask    = &waitStage,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &frame.CommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &frame.RenderFinished  // Which semaphore to signal after
        };

        // Submit frame to queue
        VK_VERIFY(m_Device->GetGraphicsQueue().submit(1, &submitInfo, frame.InFlight));
    }

    void VulkanSwapchain::PresentFrame(const VulkanFrame& frame)
    {
        // Create present info
        vk::PresentInfoKHR presentInfo
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &frame.RenderFinished,
            .swapchainCount     = 1,
            .pSwapchains        = &m_CurrentSwapchain,
            .pImageIndices      = &frame.ImageIndex
        };

        // Present
        vk::Result res = m_Device->GetGraphicsQueue().presentKHR(&presentInfo);
        if(res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || m_Resized)
        {
            LOG_WARN("vkQueuePresentKHR initialized swapchain recreation ...");
            m_Resized = false;
            RecreateSwapchain();
        }
        ASSERT(res == vk::Result::eSuccess, "Failed to present swapchain image!");
    }

    void VulkanSwapchain::AdvanceFrameCount()
    {
        m_CurrentFrame = (m_CurrentFrame + 1) % FRAMES_IN_FLIGHT;
    }

    // ----- Private -----

    void VulkanSwapchain::CreateSwapchain()
    {
        m_OldSwapchain = m_CurrentSwapchain;

        vk::SwapchainCreateInfoKHR swapchainCreate
        {
            .surface          = m_Surface,
            .minImageCount    = m_Properties.ImageCount,
            .imageFormat      = m_Properties.SurfaceFormat.format,
            .imageColorSpace  = m_Properties.SurfaceFormat.colorSpace,
            .imageExtent      = m_Properties.Extent,
            .imageArrayLayers = 1,
            .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform     = m_Properties.Transform,
            .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode      = m_Properties.PresentMode,
            .clipped          = vk::True,
            .oldSwapchain     = m_OldSwapchain
        };

        VK_VERIFY(m_Device->GetHandle().createSwapchainKHR(&swapchainCreate, nullptr, &m_CurrentSwapchain));

        LOG_INFO
        (
            "Created swapchain ... (Size: {}x{}, Format: {}, Color: {}, Mode: {})",
            m_Properties.Extent.width,
            m_Properties.Extent.height,
            vk::to_string(m_Properties.SurfaceFormat.format),
            vk::to_string(m_Properties.SurfaceFormat.colorSpace),
            vk::to_string(m_Properties.PresentMode)
        );

        if(m_OldSwapchain != nullptr)
        {
            DestroyImages();
            m_Device->GetHandle().destroySwapchainKHR(m_OldSwapchain);
        }
    }

    void VulkanSwapchain::DestroyImages()
    {
        // Destroy image views
        for(auto& image : m_Images)
        {
            m_Device->GetHandle().destroyImageView(image.View);
        }
        m_Images.clear();
    }

    void VulkanSwapchain::RecreateSwapchain()
    {
        // Wait for GPU
        m_Device->WaitForIdle();

        m_Properties = GetSwapchainProperties(m_Device->GetPhysicalDevice());
        CreateSwapchain();
        CreateImages();
    }

    void VulkanSwapchain::CreateImages()
    {
        // Retrieve image handles
        auto [result, images] = m_Device->GetHandle().getSwapchainImagesKHR(m_CurrentSwapchain);
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
                .format   = m_Properties.SurfaceFormat.format,
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
