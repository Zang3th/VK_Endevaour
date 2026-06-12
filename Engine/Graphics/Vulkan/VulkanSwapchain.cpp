#include "VulkanSwapchain.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

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

        // Specify minimum amount of images in swapchain
        properties.MinImageCount = swapchainSupport.Capabilities.minImageCount + 1;

        // Make sure to not exceed bounds (0 := means no limit)
        if (swapchainSupport.Capabilities.maxImageCount > 0
            && properties.MinImageCount > swapchainSupport.Capabilities.maxImageCount)
        {
            properties.MinImageCount = swapchainSupport.Capabilities.maxImageCount;
        }

        // Save current transform
        properties.Transform = swapchainSupport.Capabilities.currentTransform;

        return properties;
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
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        LOG_INFO("VulkanSwapchain::Destructor() ...");

        DestroyImages();

        // Destroy sync objects
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_Device->GetHandle().destroySemaphore(m_FrameResources.at(i).ImageAvailable);
            m_Device->GetHandle().destroyFence(m_FrameResources.at(i).InFlight);
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

    [[nodiscard]] std::optional<SwapchainFrame> VulkanSwapchain::BeginFrame()
    {
        // Get current frame resources
        VulkanFrameResources* currentFrameResources = &m_FrameResources.at(m_CurrentFrame);

        // Wait for this frame-slot's previous submission to finish
        VK_VERIFY(m_Device->GetHandle().waitForFences(1, &currentFrameResources->InFlight, vk::True, UINT64_MAX));

        u32 imageIndex = UINT32_MAX;

        // Try to aquire next image
        const vk::Result res = m_Device->GetHandle().acquireNextImageKHR(
            m_CurrentSwapchain, UINT64_MAX, currentFrameResources->ImageAvailable, nullptr, &imageIndex);

        if (res == vk::Result::eErrorOutOfDateKHR)
        {
            LOG_WARN("vkAcquireNextImageKHR initialized swapchain recreation ...");
            RecreateSwapchain();
            return std::nullopt;
        }

        // eSuboptimalKHR still returns a valid image. Defer recreation until present
        ASSERT(res == vk::Result::eSuccess || res == vk::Result::eSuboptimalKHR, "Failed to acquire swapchain image!");

        // Reset fence
        VK_VERIFY(m_Device->GetHandle().resetFences(1, &currentFrameResources->InFlight));

        // Reset command buffer
        VK_VERIFY(currentFrameResources->CommandBuffer.reset());

        return SwapchainFrame{ .Resources  = currentFrameResources,
                               .ImageIndex = imageIndex,
                               .Extent     = m_Properties.Extent };
    }

    void VulkanSwapchain::BeginRendering(const SwapchainFrame& frame, glm::vec4 clearColor)
    {
        // Grab shortcut handles to current frame data
        vk::CommandBuffer cmdBuffer = frame.Resources->CommandBuffer;
        SwapchainImage    image     = m_Images.at(frame.ImageIndex);

        // Start command buffer recording
        const vk::CommandBufferBeginInfo cmdBeginInfo{};
        VK_VERIFY(cmdBuffer.begin(&cmdBeginInfo));

        // Transition image layout from undefined to color
        VulkanSwapchainUtils::TransitionImageLayout(cmdBuffer,
                                                    image.Image,
                                                    vk::ImageLayout::eUndefined,
                                                    vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::AccessFlagBits2::eNone,
                                                    vk::AccessFlagBits2::eColorAttachmentWrite,
                                                    vk::PipelineStageFlagBits2::eTopOfPipe,
                                                    vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        // Set up clear value
        const vk::ClearValue clearValue{ .color = { { { clearColor.x, clearColor.y, clearColor.z, clearColor.a } } } };

        // Set up rendering attachment info
        const vk::RenderingAttachmentInfo colorAttachment{ .imageView   = image.View,
                                                           .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                                                           .loadOp      = vk::AttachmentLoadOp::eClear,
                                                           .storeOp     = vk::AttachmentStoreOp::eStore,
                                                           .clearValue  = clearValue };

        // Begin rendering
        const vk::RenderingInfo renderingInfo{ .renderArea = { .offset = { .x = 0, .y = 0 }, .extent = frame.Extent },
                                               .layerCount = 1,
                                               .colorAttachmentCount = m_Properties.ColorAttachmentCount,
                                               .pColorAttachments    = &colorAttachment };

        cmdBuffer.beginRendering(&renderingInfo);
    }

    void VulkanSwapchain::EndRendering(const SwapchainFrame& frame)
    {
        // Grab shortcut handles to current frame data
        vk::CommandBuffer cmdBuffer = frame.Resources->CommandBuffer;
        SwapchainImage    image     = m_Images.at(frame.ImageIndex);

        // Finish up rendering
        cmdBuffer.endRendering();

        // Transition image layout from color to present
        VulkanSwapchainUtils::TransitionImageLayout(cmdBuffer,
                                                    image.Image,
                                                    vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::ImageLayout::ePresentSrcKHR,
                                                    vk::AccessFlagBits2::eColorAttachmentWrite,
                                                    vk::AccessFlagBits2::eNone,
                                                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                    vk::PipelineStageFlagBits2::eBottomOfPipe);

        // End command buffer recording
        VK_VERIFY(cmdBuffer.end());
    }

    void VulkanSwapchain::SubmitAndPresent(const SwapchainFrame& frame)
    {
        // Grab shortcut handles to current frame data
        vk::CommandBuffer cmdBuffer = frame.Resources->CommandBuffer;
        SwapchainImage    image     = m_Images.at(frame.ImageIndex);

        const vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };

        // Create submit info
        const vk::SubmitInfo submitInfo{
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &frame.Resources->ImageAvailable, // On which semaphore to wait
            .pWaitDstStageMask    = &waitStage,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &cmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &image.RenderFinished // Which semaphore to signal after
        };

        // Submit frame to queue
        VK_VERIFY(m_Device->GetGraphicsQueue().submit(1, &submitInfo, frame.Resources->InFlight));

        // Advance frame count no matter if presentKHR will trigger a recreate
        AdvanceFrameCount();

        // Create present info
        const vk::PresentInfoKHR presentInfo{ .waitSemaphoreCount = 1,
                                              .pWaitSemaphores    = &image.RenderFinished,
                                              .swapchainCount     = 1,
                                              .pSwapchains        = &m_CurrentSwapchain,
                                              .pImageIndices      = &frame.ImageIndex };

        // Present
        const vk::Result res = m_Device->GetGraphicsQueue().presentKHR(&presentInfo);
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR
            || Platform::Window::GotResized())
        {
            LOG_WARN("vkQueuePresentKHR initialized swapchain recreation ...");
            RecreateSwapchain();
            Platform::Window::SetResizeFlag(false);
            return;
        }
        ASSERT(res == vk::Result::eSuccess, "Failed to present swapchain image!");
    }

    // ----- Private -----

    void VulkanSwapchain::CreateSwapchain()
    {
        m_OldSwapchain = m_CurrentSwapchain;

        const vk::SwapchainCreateInfoKHR swapchainCreate{ .surface          = m_Surface,
                                                          .minImageCount    = m_Properties.MinImageCount,
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

        LOG_WARN("Created swapchain ... (Size: {}x{}, Format: {}, Color: {}, Mode: {})",
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

        LOG_INFO("Created {} swapchain image view(s) with render-finished semaphore(s) ...", m_Images.size());
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
            VK_VERIFY(
                m_Device->GetHandle().createSemaphore(&semaphoreInfo, nullptr, &m_FrameResources[i].ImageAvailable));
            VK_VERIFY(m_Device->GetHandle().createFence(&fenceInfo, nullptr, &m_FrameResources[i].InFlight));
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
            m_FrameResources[i].CommandBuffer = commandBuffers.at(i);
        }

        LOG_INFO("Initialized sync objects and command buffers for {} frame(s)-in-flight ...", FRAMES_IN_FLIGHT);
    }

    void VulkanSwapchain::AdvanceFrameCount()
    {
        m_CurrentFrame = (m_CurrentFrame + 1) % FRAMES_IN_FLIGHT;
    }
}
