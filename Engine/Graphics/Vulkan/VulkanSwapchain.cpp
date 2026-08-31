#include "VulkanSwapchain.hpp"

#include "Debug/LogTable.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanDevice.hpp"
#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include "Platform/Window.hpp"

namespace
{
    // ----- Internal -----

    Engine::Graphics::SwapchainProperties GetSwapchainProperties(
        const Engine::Graphics::VulkanPhysicalDevice* physicalDevice)
    {
        Engine::Graphics::SwapchainProperties properties{};

        // Get swapchain capabilities from the physical device
        const Engine::Graphics::SwapchainSupport swapchainSupport = physicalDevice->GetSwapchainSupport();

        // Choose most optimal swapchain properties
        properties.Extent = Engine::Graphics::VulkanSwapchainUtils::ChooseExtent(swapchainSupport.Capabilities);
        properties.SurfaceFormat =
            Engine::Graphics::VulkanSwapchainUtils::ChooseSurfaceFormat(swapchainSupport.Formats);
        properties.PresentMode =
            Engine::Graphics::VulkanSwapchainUtils::ChoosePresentMode(swapchainSupport.PresentModes);

        properties.DepthFormat = swapchainSupport.DepthFormat;
        properties.SampleCount = swapchainSupport.SampleCount;

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

        auto deviceHandle = m_Device->GetHandle();

        // Destroy sync objects
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            deviceHandle.destroySemaphore(m_FrameResources.at(i).ImageAvailable);
            deviceHandle.destroyFence(m_FrameResources.at(i).InFlight);
        }

        // Destroy command pools
        deviceHandle.destroyCommandPool(m_GraphicsCommandPool);
        deviceHandle.destroyCommandPool(m_TransferCommandPool);

        // Destroy swapchain
        deviceHandle.destroySwapchainKHR(m_CurrentSwapchain);
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
        VulkanFrameResources* currentFrameResources = &m_FrameResources.at(m_CurrentFrameIndex);

        auto deviceHandle = m_Device->GetHandle();

        // Wait for this frame-slot's previous submission to finish
        VK_VERIFY(deviceHandle.waitForFences(1, &currentFrameResources->InFlight, vk::True, UINT64_MAX));

        u32 imageIndex = UINT32_MAX;

        // Try to aquire next image
        const vk::Result res = deviceHandle.acquireNextImageKHR(
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
        VK_VERIFY(deviceHandle.resetFences(1, &currentFrameResources->InFlight));

        // Reset command buffer
        VK_VERIFY(currentFrameResources->CommandBuffer.reset());

        return SwapchainFrame{ .Resources  = currentFrameResources,
                               .ImageIndex = imageIndex,
                               .FrameIndex = m_CurrentFrameIndex,
                               .Extent     = m_Properties.Extent };
    }

    void VulkanSwapchain::BeginRendering(const SwapchainFrame& frame, glm::vec4 clearColor)
    {
        const vk::CommandBuffer cmdBuffer = frame.Resources->CommandBuffer;

        // Start command buffer recording
        const vk::CommandBufferBeginInfo cmdBeginInfo{};
        VK_VERIFY(cmdBuffer.begin(&cmdBeginInfo));

        // Transition image layout of the internal swapchain image from undefined to color
        VulkanSwapchainUtils::TransitionImageLayout(cmdBuffer,
                                                    m_Images.at(frame.ImageIndex).InternalImage,
                                                    vk::ImageLayout::eUndefined,
                                                    vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::AccessFlagBits2::eNone,
                                                    vk::AccessFlagBits2::eColorAttachmentWrite,
                                                    vk::PipelineStageFlagBits2::eTopOfPipe,
                                                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                    vk::ImageAspectFlagBits::eColor);

        // Transition image layout of msaa color target from undefined to color
        VulkanSwapchainUtils::TransitionImageLayout(cmdBuffer,
                                                    m_Images.at(frame.ImageIndex).ColorTarget.GetImage(),
                                                    vk::ImageLayout::eUndefined,
                                                    vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::AccessFlagBits2::eNone,
                                                    vk::AccessFlagBits2::eColorAttachmentWrite,
                                                    vk::PipelineStageFlagBits2::eTopOfPipe,
                                                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                    vk::ImageAspectFlagBits::eColor);

        const vk::ClearValue clearValue{ .color = { { { clearColor.x, clearColor.y, clearColor.z, clearColor.a } } } };

        // Set up rendering attachment info for the color target
        const vk::RenderingAttachmentInfo colorAttachment{
            .imageView   = m_Images.at(frame.ImageIndex).ColorTarget.GetView(),
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .resolveMode = vk::ResolveModeFlagBits::eAverage,
            .resolveImageView =
                m_Images.at(frame.ImageIndex).InternalView, // Resolve color target in internal swapchain image (::e1)
            .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp             = vk::AttachmentLoadOp::eClear,
            .storeOp            = vk::AttachmentStoreOp::eDontCare,
            .clearValue         = clearValue
        };

        // Transition image layout of the depth target from undefined to depth
        VulkanSwapchainUtils::TransitionImageLayout(
            cmdBuffer,
            m_Images.at(frame.ImageIndex).DepthTarget.GetImage(),
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthAttachmentOptimal,
            vk::AccessFlagBits2::eNone,
            vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            vk::ImageAspectFlagBits::eDepth);

        // Set up rendering attachment info for the depth target
        const vk::RenderingAttachmentInfo depthAttachment{
            .imageView   = m_Images.at(frame.ImageIndex).DepthTarget.GetView(),
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,
            .storeOp     = vk::AttachmentStoreOp::eDontCare,
            .clearValue  = { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
        };

        // Begin rendering
        const vk::RenderingInfo renderingInfo{ .renderArea = { .offset = { .x = 0, .y = 0 }, .extent = frame.Extent },
                                               .layerCount = 1,
                                               .colorAttachmentCount = GLOBAL_COLOR_ATTACHMENT_COUNT,
                                               .pColorAttachments    = &colorAttachment,
                                               .pDepthAttachment     = &depthAttachment };

        cmdBuffer.beginRendering(&renderingInfo);
    }

    void VulkanSwapchain::EndRendering(const SwapchainFrame& frame)
    {
        const vk::CommandBuffer cmdBuffer = frame.Resources->CommandBuffer;

        // Finish up rendering
        cmdBuffer.endRendering();

        // Transition image layout from color to present
        VulkanSwapchainUtils::TransitionImageLayout(cmdBuffer,
                                                    m_Images.at(frame.ImageIndex).InternalImage,
                                                    vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::ImageLayout::ePresentSrcKHR,
                                                    vk::AccessFlagBits2::eColorAttachmentWrite,
                                                    vk::AccessFlagBits2::eNone,
                                                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                    vk::PipelineStageFlagBits2::eBottomOfPipe,
                                                    vk::ImageAspectFlagBits::eColor);

        // End command buffer recording
        VK_VERIFY(cmdBuffer.end());
    }

    void VulkanSwapchain::SubmitAndPresent(const SwapchainFrame& frame)
    {
        const vk::CommandBuffer cmdBuffer = frame.Resources->CommandBuffer;

        const vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eColorAttachmentOutput
                                                | vk::PipelineStageFlagBits::eEarlyFragmentTests };

        // Create submit info
        const vk::SubmitInfo submitInfo{
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &frame.Resources->ImageAvailable, // On which semaphore to wait
            .pWaitDstStageMask    = &waitStage,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &cmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &(m_Images.at(frame.ImageIndex).RenderFinished) // Which semaphore to signal after
        };

        // Submit frame to queue
        VK_VERIFY(m_Device->GetGraphicsQueue().submit(1, &submitInfo, frame.Resources->InFlight));

        // Advance frame count, no matter if presentKHR triggers a recreate
        AdvanceFrameCount();

        // Create present info
        const vk::PresentInfoKHR presentInfo{ .waitSemaphoreCount = 1,
                                              .pWaitSemaphores    = &(m_Images.at(frame.ImageIndex).RenderFinished),
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

        LOG_INFO("Created swapchain ...");
        LOG_TABLE_BEGIN(7);
        LOG_TABLE_COLUMN("Extent", "{}x{}", m_Properties.Extent.width, m_Properties.Extent.height);
        LOG_TABLE_COLUMN("Images", "{}", m_Properties.MinImageCount);
        LOG_TABLE_COLUMN("SurfaceFormat", "{}", vk::to_string(m_Properties.SurfaceFormat.format));
        LOG_TABLE_COLUMN("DepthFormat", "{}", vk::to_string(m_Properties.DepthFormat));
        LOG_TABLE_COLUMN("SampleCount", "{}", vk::to_string(m_Properties.SampleCount));
        LOG_TABLE_COLUMN("PresentMode", "{}", vk::to_string(m_Properties.PresentMode));
        LOG_TABLE_COLUMN("Recreated", "{}", m_OldSwapchain ? "Yes" : "No");
        LOG_TABLE_END();

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
            m_Device->GetHandle().destroyImageView(image.InternalView);
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
        auto deviceHandle = m_Device->GetHandle();

        // Retrieve image handles
        auto [result, images] = deviceHandle.getSwapchainImagesKHR(m_CurrentSwapchain);
        VK_VERIFY(result);

        // Reserve space
        m_Images.reserve(images.size());

        for (const auto& image : images)
        {
            // Create an image view for every internal image in the swapchain
            const vk::ImageViewCreateInfo viewCreateInfo = { .image            = image,
                                                             .viewType         = vk::ImageViewType::e2D,
                                                             .format           = m_Properties.SurfaceFormat.format,
                                                             .subresourceRange = { .aspectMask =
                                                                                       vk::ImageAspectFlagBits::eColor,
                                                                                   .baseMipLevel   = 0,
                                                                                   .levelCount     = 1,
                                                                                   .baseArrayLayer = 0,
                                                                                   .layerCount     = 1 } };

            auto [result, view] = deviceHandle.createImageView(viewCreateInfo);
            VK_VERIFY(result);

            const vk::SemaphoreCreateInfo semaphoreInfo{};
            auto [semaphoreResult, renderFinished] = deviceHandle.createSemaphore(semaphoreInfo);
            VK_VERIFY(semaphoreResult);

            m_Images.emplace_back(SwapchainImage{
                .ColorTarget    = VulkanRenderTarget({ .Format      = m_Properties.SurfaceFormat.format,
                                                       .Extent      = m_Properties.Extent,
                                                       .SampleCount = m_Properties.SampleCount,
                                                       .UsageFlags  = vk::ImageUsageFlagBits::eColorAttachment,
                                                       .AspectFlags = vk::ImageAspectFlagBits::eColor,
                                                       .DebugName   = "ColorTarget" },
                                                     deviceHandle),
                .DepthTarget    = VulkanRenderTarget({ .Format      = m_Properties.DepthFormat,
                                                       .Extent      = m_Properties.Extent,
                                                       .SampleCount = m_Properties.SampleCount,
                                                       .UsageFlags  = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                                       .AspectFlags = vk::ImageAspectFlagBits::eDepth,
                                                       .DebugName   = "DepthTarget" },
                                                     deviceHandle),
                .InternalImage  = image,
                .InternalView   = view,
                .RenderFinished = renderFinished });
        }

        LOG_INFO("Created {} image(s) with view(s) and render-finished semaphore(s) ...", m_Images.size());
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

        auto deviceHandle = m_Device->GetHandle();

        // Create sync objects
        for (u32 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VK_VERIFY(deviceHandle.createSemaphore(&semaphoreInfo, nullptr, &m_FrameResources[i].ImageAvailable));
            VK_VERIFY(deviceHandle.createFence(&fenceInfo, nullptr, &m_FrameResources[i].InFlight));
        }

        // Allocate command buffers
        const vk::CommandBufferAllocateInfo allocateInfo{ .commandPool        = m_GraphicsCommandPool,
                                                          .level              = vk::CommandBufferLevel::ePrimary,
                                                          .commandBufferCount = FRAMES_IN_FLIGHT };
        auto [res, commandBuffers] = deviceHandle.allocateCommandBuffers(allocateInfo);
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
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
    }
}
