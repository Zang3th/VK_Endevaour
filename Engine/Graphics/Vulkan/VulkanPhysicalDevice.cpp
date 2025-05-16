#include "VulkanPhysicalDevice.hpp"
#include "VulkanAssert.hpp"
#include "VulkanGlobals.hpp"

#include "Debug/Log.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <set>

namespace
{
    // ----- Internal -----

    inline std::string GetDriverVersionString(vk::PhysicalDeviceProperties properties)
    {
        if(properties.vendorID != 0x10DE) // Nvidia
        {
            LOG_WARN("GPU vendor isn't Nvidia. Driver version string may be false ...");
        }

        u32 major     = (properties.driverVersion >> 22) & 0x3ff;
        u32 minor     = (properties.driverVersion >> 14) & 0x0ff;
        u32 secondary = (properties.driverVersion >> 6)  & 0x0ff;
        u32 tertiary  = properties.driverVersion & 0x3f;

        return fmt::format("{}.{}.{}.{}", major, minor, secondary, tertiary);
    }
}

namespace Engine
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

    void VulkanPhysicalDevice::PickDevice()
    {
        // Query for devices
        const auto [result, devices] = m_Instance.enumeratePhysicalDevices();
        VK_VERIFY(result);
        ASSERT(!devices.empty(), "Failed to find GPUs with Vulkan support!");

        LOG_INFO("Check for suitable device ...");

        // Check each device for suitability
        for(const auto& device : devices)
        {
            // Break at first suitable device
            if(IsDeviceSuitable(device))
            {
                m_PhysicalDevice = device;
                m_Properties = device.getProperties();
                LOG_INFO("Found suitable device ... (GPU: {}, Driver: {})",
                        (const char*)m_Properties.deviceName, GetDriverVersionString(m_Properties));
                break;
            }
        }

        ASSERT(m_PhysicalDevice, "Failed to find suitable device!");
    }

    // ----- Private -----

    bool VulkanPhysicalDevice::IsDeviceSuitable(vk::PhysicalDevice device)
    {
        m_QueueFamilyIndices = FindQueueFamilyIndices(device);

        return m_QueueFamilyIndices.isComplete()          &&
               QuerySwapchainSupport(device).isComplete() &&
               CheckDeviceExtensionSupport(device);
    }

    QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilyIndices(vk::PhysicalDevice device)
    {
        QueueFamilyIndices queueFamilyIndices;
        VkBool32 presentSupport = false;
        u32 index = 0;

        // Query for queue families
        const auto queueFamilies = device.getQueueFamilyProperties();

        // Iterate over queue familys
        for(const auto& queueFamily : queueFamilies)
        {
            // Query for graphics capable queue family
            if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                // Query for capability of presenting to a window surface
                VK_VERIFY((vk::Result)device.getSurfaceSupportKHR(index, m_Surface, &presentSupport));

                if(presentSupport)
                {
                    queueFamilyIndices.GraphicsFamily = index;
                }

                // If no dedicated transfer queue was found, use graphics queue for transfer
                if(queueFamilyIndices.TransferFamily < 0)
                {
                    queueFamilyIndices.TransferFamily = index;
                }
            }
            // Query for dedicated transfer queue
            else if(queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                queueFamilyIndices.TransferFamily = index;
            }

            index++;
        }

        return queueFamilyIndices;
    }

    bool VulkanPhysicalDevice::CheckDeviceExtensionSupport(vk::PhysicalDevice device)
    {
        // Query for available extensions
        const auto [result, availableExtensions] = device.enumerateDeviceExtensionProperties();
        VK_VERIFY(result);

        // Package globally defined device extensions
        std::set<std::string> requiredExtensions(g_DeviceExtensions.begin(), g_DeviceExtensions.end());

        // Delete if available
        for(const auto& extension : availableExtensions)
        {
            if(requiredExtensions.contains(extension.extensionName))
            {
                LOG_INFO("Found support for required device extension: {} ...", extension.extensionName.data());
                requiredExtensions.erase(extension.extensionName);
            }
        }

        if(!requiredExtensions.empty())
        {
            for(const auto& ext : requiredExtensions)
            {
                LOG_WARN("Missing required device extension: {}", ext);
            }
        }

        return requiredExtensions.empty();
    }

    [[nodiscard]] SwapchainSupport VulkanPhysicalDevice::QuerySwapchainSupport(vk::PhysicalDevice physicalDevice) const
    {
        SwapchainSupport swapchainSupport;

        {
            auto [result, capabilities] = physicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
            VK_VERIFY(result);
            swapchainSupport.Capabilities = capabilities;
        }
        {
            auto [result, surfaceFormats] = physicalDevice.getSurfaceFormatsKHR(m_Surface);
            VK_VERIFY(result);
            swapchainSupport.Formats = std::move(surfaceFormats);
        }
        {
            auto [result, presentModes] = physicalDevice.getSurfacePresentModesKHR(m_Surface);
            VK_VERIFY(result);
            swapchainSupport.PresentModes = std::move(presentModes);
        }

        return swapchainSupport;
    }
}
