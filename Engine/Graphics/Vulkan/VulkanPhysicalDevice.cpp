#include "VulkanPhysicalDevice.hpp"

#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <array>
#include <set>

namespace
{
    // ----- Internal -----

    [[nodiscard]] std::string GetDriverVersionString(const vk::PhysicalDeviceProperties& properties)
    {
        if (properties.vendorID != 0x10DE) // Nvidia
        {
            LOG_WARN("GPU vendor isn't Nvidia. Driver version string may be false ...");
        }

        Engine::u32 major     = (properties.driverVersion >> 22) & 0x3ff;
        Engine::u32 minor     = (properties.driverVersion >> 14) & 0x0ff;
        Engine::u32 secondary = (properties.driverVersion >> 6) & 0x0ff;
        Engine::u32 tertiary  = properties.driverVersion & 0x3f;

        return fmt::format("{}.{}.{}.{}", major, minor, secondary, tertiary);
    }

    [[nodiscard]] bool CheckExtensionSupport(const vk::PhysicalDevice& device)
    {
        // Query for available extensions
        const auto [result, availableExtensions] = device.enumerateDeviceExtensionProperties();
        VK_VERIFY(result);

        // Package globally defined device extensions
        std::set<std::string> requiredExtensions(Engine::Graphics::g_DeviceExtensions.begin(),
                                                 Engine::Graphics::g_DeviceExtensions.end());

        // Delete if available
        for (const auto& extension : availableExtensions)
        {
            if (requiredExtensions.contains(extension.extensionName))
            {
                LOG_INFO("Found support for required device extension: {} ...", extension.extensionName.data());
                requiredExtensions.erase(extension.extensionName);
            }
        }

        if (!requiredExtensions.empty())
        {
            for (const auto& ext : requiredExtensions)
            {
                LOG_WARN("Missing required device extension: {}", ext);
            }
        }

        return requiredExtensions.empty();
    }

    [[nodiscard]] Engine::Graphics::QueueFamilyIndices FindQueueFamilyIndices(const vk::PhysicalDevice& device,
                                                                              const vk::SurfaceKHR&     surface)
    {
        Engine::Graphics::QueueFamilyIndices queueFamilyIndices;
        VkBool32                             presentSupport = false;
        Engine::u32                          index          = 0;

        // Query for queue families
        const auto queueFamilies = device.getQueueFamilyProperties();

        // Iterate over queue familys
        for (const auto& queueFamily : queueFamilies)
        {
            // Query for graphics capable queue family
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                // Query for capability of presenting to a window surface
                VK_VERIFY((vk::Result)device.getSurfaceSupportKHR(index, surface, &presentSupport));

                if (presentSupport)
                {
                    queueFamilyIndices.GraphicsFamily = index;
                }

                // If no dedicated transfer queue was found, use graphics queue for transfer
                if (!queueFamilyIndices.HasTransferFamily())
                {
                    queueFamilyIndices.TransferFamily = queueFamilyIndices.GraphicsFamily;
                }
            }
            // Query for dedicated transfer queue
            else if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                queueFamilyIndices.TransferFamily = index;
            }

            index++;
        }

        return queueFamilyIndices;
    }

    [[nodiscard]] vk::Format FindSupportedDepthFormat(vk::PhysicalDevice device)
    {
        constexpr std::array formats = {
            vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16Unorm
        };

        constexpr vk::FormatFeatureFlagBits required = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

        for (const vk::Format format : formats)
        {
            const vk::FormatProperties properties = device.getFormatProperties(format);

            if ((properties.optimalTilingFeatures & required) == required)
            {
                return format;
            }
        }

        return vk::Format::eUndefined;
    }

    [[nodiscard]] vk::SampleCountFlagBits FindSupportedSampleCount(vk::PhysicalDevice device)
    {
        auto properties = device.getProperties();
        auto supportedSampleCounts =
            properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

        if (supportedSampleCounts & vk::SampleCountFlagBits::e4)
        {
            return vk::SampleCountFlagBits::e4;
        }

        if (supportedSampleCounts & vk::SampleCountFlagBits::e2)
        {
            return vk::SampleCountFlagBits::e2;
        }

        return vk::SampleCountFlagBits::e1;
    }
}

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanPhysicalDevice::VulkanPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface)
        : m_Instance(instance), m_Surface(surface)
    {
        PickDevice();
    }

    VulkanPhysicalDevice::~VulkanPhysicalDevice()
    {
        LOG_INFO("VulkanPhysicalDevice::Destructor() ...");
    }

    // ----- Private -----

    void VulkanPhysicalDevice::PickDevice()
    {
        // Query for devices
        const auto [result, devices] = m_Instance.enumeratePhysicalDevices();
        VK_VERIFY(result);
        ASSERT(!devices.empty(), "Failed to find GPUs with Vulkan support!");

        LOG_INFO("Check for suitable device ...");

        // Check each device for suitability
        for (const auto& device : devices)
        {
            const QueueFamilyIndices queueFamilies    = FindQueueFamilyIndices(device, m_Surface);
            const SwapchainSupport   swapchainSupport = QuerySwapchainSupport(device, m_Surface);

            // Break at first suitable device
            if (queueFamilies.IsComplete() && swapchainSupport.IsComplete() && CheckExtensionSupport(device))
            {
                m_PhysicalDevice     = device;
                m_QueueFamilyIndices = queueFamilies;
                m_Properties         = device.getProperties();
                LOG_INFO("Found suitable device ... (GPU: {}, Driver: {})",
                         (const char*)m_Properties.deviceName,
                         GetDriverVersionString(m_Properties));
                break;
            }
        }

        ASSERT(m_PhysicalDevice, "Failed to find suitable device!");
    }

    [[nodiscard]] SwapchainSupport VulkanPhysicalDevice::QuerySwapchainSupport(const vk::PhysicalDevice& device,
                                                                               const vk::SurfaceKHR&     surface) const
    {
        SwapchainSupport swapchainSupport;

        {
            auto [result, capabilities] = device.getSurfaceCapabilitiesKHR(surface);
            VK_VERIFY(result);
            swapchainSupport.Capabilities = capabilities;
        }
        {
            auto [result, surfaceFormats] = device.getSurfaceFormatsKHR(surface);
            VK_VERIFY(result);
            swapchainSupport.Formats = std::move(surfaceFormats);
        }
        {
            auto [result, presentModes] = device.getSurfacePresentModesKHR(surface);
            VK_VERIFY(result);
            swapchainSupport.PresentModes = std::move(presentModes);
        }

        // It's okay for now to check these here, even though they are hardware locked and don't change
        swapchainSupport.DepthFormat = FindSupportedDepthFormat(device);

        // Same thing here ...
        swapchainSupport.SampleCount = FindSupportedSampleCount(device);

        return swapchainSupport;
    }
}
