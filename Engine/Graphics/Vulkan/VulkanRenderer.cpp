#include "VulkanRenderer.hpp"
#include "VulkanAssert.hpp"

#include "Debug/Log.hpp"

namespace
{
    // TODO: Move into swapchain
    // Copied from Khronos 'hello_triangle_1_3.cpp' example
    void TransitionImageLayout
    (
        vk::CommandBuffer       cmd,
        vk::Image               image,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags2        srcAccessMask,
        vk::AccessFlags2        dstAccessMask,
        vk::PipelineStageFlags2 srcStage,
        vk::PipelineStageFlags2 dstStage
    )
    {
        // Initialize the VkImageMemoryBarrier2 structure
        vk::ImageMemoryBarrier2 imageMemoryBarrier
        {
            // Specify the pipeline stages and access masks for the barrier
            .srcStageMask  = srcStage,             // Source pipeline stage mask
            .srcAccessMask = srcAccessMask,        // Source access mask
            .dstStageMask  = dstStage,             // Destination pipeline stage mask
            .dstAccessMask = dstAccessMask,        // Destination access mask

            // Specify the old and new layouts of the image
            .oldLayout = oldLayout,        // Current layout of the image
            .newLayout = newLayout,        // Target layout of the image

            // We are not changing the ownership between queues
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

            // Specify the image to be affected by this barrier
            .image = image,

            // Define the subresource range (which parts of the image are affected)
            .subresourceRange =
            {
                .aspectMask     = vk::ImageAspectFlagBits::eColor,  // Affects the color aspect of the image
                .baseMipLevel   = 0,                                // Start at mip level 0
                .levelCount     = 1,                                // Number of mip levels affected
                .baseArrayLayer = 0,                                // Start at array layer 0
                .layerCount     = 1                                 // Number of array layers affected
            }
        };

        // Initialize the VkDependencyInfo structure
        vk::DependencyInfo dependencyInfo
        {
            .imageMemoryBarrierCount = 1,                    // Number of image memory barriers
            .pImageMemoryBarriers    = &imageMemoryBarrier   // Pointer to the image memory barrier(s)
        };

        // Record the pipeline barrier into the command buffer
        cmd.pipelineBarrier2(&dependencyInfo);
    }
}

namespace Engine
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
        : m_ShaderIndex(0), m_ModelIndex(0), m_PipelineIndex(0), m_DrawcallCount(0)
    {
        m_Context = MakeScope<VulkanContext>();
    }

    [[nodiscard]] u32 VulkanRenderer::LoadShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path)
    {
        ASSERT(m_ShaderIndex != MAX_SHADER_COUNT, "Reached capacity ... Can't load any more shaders!");

        u32 currentIndex = m_ShaderIndex;
        m_Shaders.at(currentIndex) = MakeScope<VulkanShader>(m_Context->GetDevice()->GetHandle(), stage, path);
        m_ShaderIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::CreateModel(const Mesh* mesh)
    {
        ASSERT(m_ModelIndex != MAX_MODEL_COUNT, "Reached capacity ... Can't load any more models!");

        u32 currentIndex = m_ModelIndex;
        m_Models.at(currentIndex) = MakeScope<VulkanModel>(m_Context.get(), mesh);
        m_ModelIndex++;

        return currentIndex;
    }

    [[nodiscard]] u32 VulkanRenderer::CreatePipeline(u32 vertexID, u32 fragmentID)
    {
        ASSERT(m_PipelineIndex != MAX_PIPELINE_COUNT, "Reached capacity ... Can't create any more pipelines!");

        u32 currentIndex = m_PipelineIndex;
        PipelineSpecification spec
        {
            .VertexShader   = m_Shaders.at(vertexID).get(),
            .FragmentShader = m_Shaders.at(fragmentID).get(),
            .DepthTest      = vk::False,
            .DepthWrite     = vk::False
        };
        m_Pipelines[currentIndex] = MakeScope<VulkanPipeline>(m_Context.get(), spec);
        m_PipelineIndex++;

        return currentIndex;
    }

    void VulkanRenderer::AssignPipeline(u32 modelID, u32 pipelineID)
    {
        m_Models.at(modelID)->AssignPipeline(pipelineID);
        LOG_INFO("Bound model '{}' to pipeline '{}' ...", modelID, pipelineID);
    }

    void VulkanRenderer::DrawFrame(u32 pipelineID)
    {
        // Reset drawcall counter for this frame
        m_DrawcallCount = 0;

        // Get current frame from the swapchain
        auto [renderNextFrame, frame] = m_Context->GetSwapchain()->GetCurrentFrame();
        if(!renderNextFrame)
        {
            return;
        }

        // Start command buffer recording
        vk::CommandBufferBeginInfo cmdBeginInfo{};
        VK_VERIFY(frame.CommandBuffer.begin(&cmdBeginInfo));

        // TODO: Move into swapchain
        // Transition image layout from undefined to color
        TransitionImageLayout
        (
            frame.CommandBuffer,
            m_Context->GetSwapchain()->GetCurrentImage().Image,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::AccessFlagBits2::eNone,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        // Set up clear value
        vk::ClearValue clearValue { .color = {{{ 1.0f, 1.0f, 1.0f, 1.0f }}}};

        // Set up rendering attachment info
        vk::RenderingAttachmentInfo colorAttachment
        {
            .imageView   = m_Context->GetSwapchain()->GetCurrentImage().View,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,
            .storeOp     = vk::AttachmentStoreOp::eStore,
            .clearValue  = clearValue
        };

        // Begin rendering
        vk::RenderingInfo renderingInfo
        {
            .renderArea = { .offset = {0, 0}, .extent = m_Context->GetSwapchain()->GetExtent() },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment
        };
        // TODO: Move into swapchain
        frame.CommandBuffer.beginRendering(&renderingInfo);

        // Set dynamic states
        const vk::Viewport viewport = m_Context->GetSwapchain()->GetViewport();
        const vk::Rect2D   scissor  = m_Context->GetSwapchain()->GetScissor();
        frame.CommandBuffer.setViewport(0, 1, &viewport);
        frame.CommandBuffer.setScissor(0, 1, &scissor);
        frame.CommandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill, m_Context->GetLoader());
        frame.CommandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
        frame.CommandBuffer.setCullMode(vk::CullModeFlagBits::eBack);
        frame.CommandBuffer.setFrontFace(vk::FrontFace::eClockwise);

        // Bind pipeline
        m_Pipelines.at(pipelineID)->Bind(frame.CommandBuffer);

        // Draw all models assigned to this pipeline
        for(u32 i = 0; i < m_ModelIndex; i++)
        {
            // Get model
            const VulkanModel* model = m_Models.at(i).get();

            // Check for pipeline
            if(model->GetPipelineID() == pipelineID)
            {
                // Bind model
                model->Bind(frame.CommandBuffer);

                // Issue draw call
                frame.CommandBuffer.drawIndexed(model->GetIndexCount(), 1, 0, 0, 0);

                // Increment drawcall counter
                m_DrawcallCount++;
            }
        }

        // TODO: Move into swapchain
        // Complete rendering
        frame.CommandBuffer.endRendering();

        // TODO: Move into swapchain
        // Transition image layout from color to present
        TransitionImageLayout
        (
            frame.CommandBuffer,
            m_Context->GetSwapchain()->GetCurrentImage().Image,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );

        // End command buffer recording
        VK_VERIFY(frame.CommandBuffer.end());

        // Submit frame
        m_Context->GetSwapchain()->SubmitFrame(frame);

        // Present image
        m_Context->GetSwapchain()->PresentFrame(frame);

        // Increment frame
        m_Context->GetSwapchain()->AdvanceFrame();
    }

    void VulkanRenderer::WaitForDevice()
    {
        m_Context->GetDevice()->WaitForIdle();
    }
}
