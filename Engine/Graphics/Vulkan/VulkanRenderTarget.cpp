#include "VulkanRenderTarget.hpp"

#include "Graphics/Vulkan/VulkanAllocator.hpp"
#include "Graphics/Vulkan/VulkanAssert.hpp"

#include <utility>

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanRenderTarget::VulkanRenderTarget(const VulkanRenderTargetCreateInfo& createInfo, const vk::Device& device)
        : m_Device(device)
    {
        Create(createInfo);
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
        Destroy();
    }

    VulkanRenderTarget::VulkanRenderTarget(VulkanRenderTarget&& other) noexcept
        : m_Device(std::exchange(other.m_Device, nullptr)),
          m_Image(std::exchange(other.m_Image, nullptr)),
          m_View(std::exchange(other.m_View, nullptr)),
          m_ImageAllocation(std::exchange(other.m_ImageAllocation, {}))
    {
    }

    VulkanRenderTarget& VulkanRenderTarget::operator=(VulkanRenderTarget&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        Destroy();

        m_Device          = std::exchange(other.m_Device, nullptr);
        m_Image           = std::exchange(other.m_Image, nullptr);
        m_View            = std::exchange(other.m_View, nullptr);
        m_ImageAllocation = std::exchange(other.m_ImageAllocation, {});

        return *this;
    }

    // ----- Private -----

    void VulkanRenderTarget::Create(const VulkanRenderTargetCreateInfo& createInfo)
    {
        const vk::ImageCreateInfo imageCreateInfo{
            .imageType   = vk::ImageType::e2D,
            .format      = createInfo.Format,
            .extent      = { .width = createInfo.Extent.width, .height = createInfo.Extent.height, .depth = 1 },
            .mipLevels   = 1,
            .arrayLayers = 1,
            .samples     = createInfo.SampleCount,
            .tiling      = vk::ImageTiling::eOptimal,
            .usage       = createInfo.UsageFlags,
            .sharingMode = vk::SharingMode::eExclusive,
        };

        // Create and allocate image
        const ImageSpecification imageSpec{
            .CreateInfo  = &imageCreateInfo,
            .Image       = &m_Image,
            .MemoryUsage = MemoryUsage::eAutoPreferDevice,
            .DebugName   = createInfo.DebugName,
        };
        m_ImageAllocation = Engine::Graphics::VulkanAllocator::AllocateImage(imageSpec);

        const vk::ImageViewCreateInfo viewCreateInfo = { .image            = m_Image,
                                                         .viewType         = vk::ImageViewType::e2D,
                                                         .format           = createInfo.Format,
                                                         .subresourceRange = { .aspectMask     = createInfo.AspectFlags,
                                                                               .baseMipLevel   = 0,
                                                                               .levelCount     = 1,
                                                                               .baseArrayLayer = 0,
                                                                               .layerCount     = 1 } };

        // Create view
        auto [result, view] = m_Device.createImageView(viewCreateInfo);
        VK_VERIFY(result);
        m_View = view;

        LOG_INFO("Created render target ({}) ...", vk::to_string(createInfo.Format));
    }

    void VulkanRenderTarget::Destroy()
    {
        // Completely empty is a valid state
        if (m_Device == nullptr && m_Image == nullptr && m_View == nullptr && m_ImageAllocation.Handle == nullptr)
        {
            return;
        }

        ASSERT(m_Device != nullptr, "RenderTarget has no valid device!");
        ASSERT(m_View != nullptr, "RenderTarget has no valid image view to destroy!");
        m_Device.destroyImageView(m_View, nullptr);
        LOG_INFO("Destroyed image view for render target ...");

        ASSERT(m_Image != nullptr, "RenderTarget has no valid image to destroy!");
        ASSERT(m_ImageAllocation.Handle != nullptr, "RenderTarget holds no valid allocation handle!");
        VulkanAllocator::DestroyImage(m_Image, m_ImageAllocation);

        m_Device          = nullptr;
        m_Image           = nullptr;
        m_View            = nullptr;
        m_ImageAllocation = {};
    }
}
