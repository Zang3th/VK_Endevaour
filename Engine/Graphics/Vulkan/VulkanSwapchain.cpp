#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanSwapchain.hpp"

#include "Platform/Window.hpp"

namespace
{
    // ----- Internal -----

    Engine::Graphics::SwapchainProperties GetSwapchainProperties(
        const Engine::Graphics::VulkanPhysicalDevice* physicalDevice)
    {
        Engine::Graphics::SwapchainProperties properties{};

        // Get swapchain capabilites from the physical device
        const Engine::Graphics::SwapchainSupport swapchainSupport = physicalDevice->GetSwapchainSupport();

        // Choose most optimal swapchain properties
        properties.Extent = Engine::Graphics::VulkanSwapchainUtils::ChooseExtent(swapchainSupport.Capabilities);
        properties.SurfaceFormat =
            Engine::Graphics::VulkanSwapchainUtils::ChooseSurfaceFormat(swapchainSupport.Formats);
        properties.PresentMode =
            Engine::Graphics::VulkanSwapchainUtils::ChoosePresentMode(swapchainSupport.PresentModes);

        // Specify amount of images in swapchain
        properties.ImageCount = swapchainSupport.Capabilities.minImageCount + 1;

        // Make sure to not exceed bounds (0 := means no limit)
        if (swapchainSupport.Capabilities.maxImageCount > 0
            && properties.ImageCount > swapchainSupport.Capabilities.maxImageCount)
        {
            properties.ImageCount = swapchainSupport.Capabilities.maxImageCount;
        }

        // Save current transform
        properties.Transform = swapchainSupport.Capabilities.currentTransform;

        return properties;
    }

    void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        // Only one window is supported
        ASSERT(window == Engine::Platform::Window::GetHandle());

        auto* swapchain = (Engine::Graphics::VulkanSwapchain*)glfwGetWindowUserPointer(window);

        if (width == 0 || height == 0)
        {
            Engine::Platform::Window::SetMinimize(true);
            LOG_INFO("GLFW::FramebufferResizeCallback(): Window got minimized ...");
            return;
        }

        if (Engine::Platform::Window::IsMinimized())
        {
            // Reset minimize
            Engine::Platform::Window::SetMinimize(false);
            LOG_INFO("GLFW::FramebufferResizeCallback(): Window no longer minimized ...");
        }

        if (swapchain->GetProperties().Extent.width != (Engine::u32)width
            || swapchain->GetProperties().Extent.height != (Engine::u32)height)
        {
            LOG_WARN("GLFW::FramebufferResizeCallback: Window got resized ... ({}x{})", width, height);
            Engine::Platform::Window::SetWidth((Engine::u32)width);
            Engine::Platform::Window::SetHeight((Engine::u32)height);
            swapchain->SetResizeFlag();
        }
    }
}

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanSwapchain::VulkanSwapchain(const VulkanDevice* device, const vk::SurfaceKHR& surface)
        : m_Device(device), m_Surface(surface)
    {
        m_Properties = GetSwapchainProperties(device->GetPhysicalDevice());
        CreateCommandPools();
        InitializeFrames();
        CreateSwapchain();
        CreateImages();

        // Set framebuffer resize callback
        glfwSetWindowUserPointer(Platform::Window::GetHandle(), this);
        glfwSetFramebufferSizeCallback(Platform::Window::GetHandle(), FramebufferResizeCallback);
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        LOG_INFO("VulkanSwapchain::Destructor() ...");

        DestroyImages();

        // Destroy sync objects
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_Device->GetHandle().destroySemaphore(m_Frames.at(i).ImageAvailable);
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
        const vk::CommandBufferAllocateInfo allocateInfo{ .commandPool        = m_TransferCommandPool,
                                                          .level              = vk::CommandBufferLevel::ePrimary,
                                                          .commandBufferCount = 1 };
        auto [res, commandBuffer] = m_Device->GetHandle().allocateCommandBuffers(allocateInfo);
        VK_VERIFY(res);
        ASSERT(!commandBuffer.empty(), "Allocated command buffer vector was empty!");

        // Begin command buffer recording
        const vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
        VK_VERIFY(commandBuffer.at(0).begin(&beginInfo));

        return commandBuffer.at(0);
    }

    void VulkanSwapchain::SubmitTransferCommandBuffer(vk::CommandBuffer commandBuffer)
    {
        // End command buffer recording
        VK_VERIFY(commandBuffer.end());

        // Immediately submit recorded commands
        const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &commandBuffer };
        const vk::Queue      transferQueue = m_Device->GetTransferQueue();
        VK_VERIFY(transferQueue.submit(1, &submitInfo, nullptr));
        VK_VERIFY(transferQueue.waitIdle());

        // Clean up command buffer
        m_Device->GetHandle().freeCommandBuffers(m_TransferCommandPool, 1, &commandBuffer);
    }

    [[nodiscard]] VulkanFrame& VulkanSwapchain::GetCurrentFrame()
    {
        // Get current frame
        VulkanFrame& currentFrame = m_Frames.at(m_CurrentFrame);

        // Wait for this frame-slot's previous submission to finish
        VK_VERIFY(m_Device->GetHandle().waitForFences(1, &currentFrame.InFlight, vk::True, UINT64_MAX));

        // Return frame
        return currentFrame;
    }

    [[nodiscard]] std::optional<u32> VulkanSwapchain::AcquireImage(const VulkanFrame& frame)
    {
        u32 imageIndex = UINT32_MAX;

        const vk::Result res = m_Device->GetHandle().acquireNextImageKHR(
            m_CurrentSwapchain, UINT64_MAX, frame.ImageAvailable, nullptr, &imageIndex);

        if (res == vk::Result::eErrorOutOfDateKHR)
        {
            LOG_WARN("vkAcquireNextImageKHR initialized swapchain recreation ...");
            RecreateSwapchain();
            return std::nullopt;
        }

        // eSuboptimalKHR still returns a valid image. Defer recreation until present
        ASSERT(res == vk::Result::eSuccess || res == vk::Result::eSuboptimalKHR, "Failed to acquire swapchain image!");

        return imageIndex;
    }

    void VulkanSwapchain::ResetFrame(const VulkanFrame& frame)
    {
        // Reset fence
        VK_VERIFY(m_Device->GetHandle().resetFences(1, &frame.InFlight));

        // Reset command buffer
        VK_VERIFY(frame.CommandBuffer.reset());
    }

    void VulkanSwapchain::SubmitFrame(const VulkanFrame& frame, u32 imageIndex)
    {
        const vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };

        // Create submit info
        const vk::SubmitInfo submitInfo{
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &frame.ImageAvailable, // On which semaphore to wait
            .pWaitDstStageMask    = &waitStage,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &frame.CommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &m_Images.at(imageIndex).RenderFinished // Which semaphore to signal after
        };

        // Submit frame to queue
        VK_VERIFY(m_Device->GetGraphicsQueue().submit(1, &submitInfo, frame.InFlight));
    }

    void VulkanSwapchain::PresentFrame(u32 imageIndex)
    {
        // Create present info
        const vk::PresentInfoKHR presentInfo{ .waitSemaphoreCount = 1,
                                              .pWaitSemaphores    = &m_Images.at(imageIndex).RenderFinished,
                                              .swapchainCount     = 1,
                                              .pSwapchains        = &m_CurrentSwapchain,
                                              .pImageIndices      = &imageIndex };

        // Present
        const vk::Result res = m_Device->GetGraphicsQueue().presentKHR(&presentInfo);
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || m_Resized)
        {
            LOG_WARN("vkQueuePresentKHR initialized swapchain recreation ...");
            m_Resized = false;
            RecreateSwapchain();
            return;
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

        const vk::SwapchainCreateInfoKHR swapchainCreate{ .surface          = m_Surface,
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
                                                          .oldSwapchain     = m_OldSwapchain };

        VK_VERIFY(m_Device->GetHandle().createSwapchainKHR(&swapchainCreate, nullptr, &m_CurrentSwapchain));

        LOG_INFO("Created swapchain ... (Size: {}x{}, Format: {}, Color: {}, Mode: {})",
                 m_Properties.Extent.width,
                 m_Properties.Extent.height,
                 vk::to_string(m_Properties.SurfaceFormat.format),
                 vk::to_string(m_Properties.SurfaceFormat.colorSpace),
                 vk::to_string(m_Properties.PresentMode));

        if (m_OldSwapchain != nullptr)
        {
            DestroyImages();
            m_Device->GetHandle().destroySwapchainKHR(m_OldSwapchain);
        }
    }

    void VulkanSwapchain::DestroyImages()
    {
        // Destroy image views
        for (auto& image : m_Images)
        {
            m_Device->GetHandle().destroyImageView(image.View);
            m_Device->GetHandle().destroySemaphore(image.RenderFinished);
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
        for (const auto& image : images)
        {
            const vk::ImageViewCreateInfo viewCreateInfo = { .image            = image,
                                                             .viewType         = vk::ImageViewType::e2D,
                                                             .format           = m_Properties.SurfaceFormat.format,
                                                             .subresourceRange = { .aspectMask =
                                                                                       vk::ImageAspectFlagBits::eColor,
                                                                                   .baseMipLevel   = 0,
                                                                                   .levelCount     = 1,
                                                                                   .baseArrayLayer = 0,
                                                                                   .layerCount     = 1 } };

            auto [result, view] = m_Device->GetHandle().createImageView(viewCreateInfo);
            VK_VERIFY(result);

            const vk::SemaphoreCreateInfo semaphoreInfo{};
            auto [semaphoreResult, renderFinished] = m_Device->GetHandle().createSemaphore(semaphoreInfo);
            VK_VERIFY(semaphoreResult);

            m_Images.emplace_back(SwapchainImage{ .Image = image, .View = view, .RenderFinished = renderFinished });
        }

        LOG_INFO("Created {} swapchain image view(s) and render-finished semaphore(s) ...", images.size());
    }

    void VulkanSwapchain::CreateCommandPools()
    {
        // Create graphics pool
        {
            const vk::CommandPoolCreateInfo graphicsPoolInfo{ .flags =
                                                                  vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                              .queueFamilyIndex = m_Device->GetGraphicsQueueFamily() };
            auto [res, pool] = m_Device->GetHandle().createCommandPool(graphicsPoolInfo);
            VK_VERIFY(res);
            m_GraphicsCommandPool = pool;
        }

        // Create transfer pool
        {
            const vk::CommandPoolCreateInfo transferPoolInfo{
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
        const vk::SemaphoreCreateInfo semaphoreInfo{};

        // Define fence (in signaled state to avoid endless waiting for the first frame)
        const vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

        // Create sync objects
        for (u32 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VK_VERIFY(m_Device->GetHandle().createSemaphore(&semaphoreInfo, nullptr, &m_Frames[i].ImageAvailable));
            VK_VERIFY(m_Device->GetHandle().createFence(&fenceInfo, nullptr, &m_Frames[i].InFlight));
        }

        // Allocate command buffers
        const vk::CommandBufferAllocateInfo allocateInfo{ .commandPool        = m_GraphicsCommandPool,
                                                          .level              = vk::CommandBufferLevel::ePrimary,
                                                          .commandBufferCount = FRAMES_IN_FLIGHT };
        auto [res, commandBuffers] = m_Device->GetHandle().allocateCommandBuffers(allocateInfo);
        VK_VERIFY(res);
        ASSERT(!commandBuffers.empty(), "Allocated command buffer vector was empty!");

        // Copy command buffers in frame struct
        for (u32 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_Frames[i].CommandBuffer = commandBuffers.at(i);
        }

        LOG_INFO("Initialized sync objects and command buffers for {} frame(s)-in-flight ...", FRAMES_IN_FLIGHT);
    }
}
