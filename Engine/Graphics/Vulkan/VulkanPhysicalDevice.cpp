#include "VulkanPhysicalDevice.hpp"
#include "VulkanAssert.hpp"
#include "Debug/Log.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace
{
    // ----- Internal -----

    inline std::string GetDriverVersionString(vk::PhysicalDeviceProperties properties)
    {
        if(properties.vendorID != 0x10DE) // Nvidia
        {
            LOG_WARN("GPU vendor isn't Nvidia. Driver version may be false ...");
        }

        uint32_t major     = (properties.driverVersion >> 22) & 0x3ff;
        uint32_t minor     = (properties.driverVersion >> 14) & 0x0ff;
        uint32_t secondary = (properties.driverVersion >> 6)  & 0x0ff;
        uint32_t tertiary  = properties.driverVersion & 0x3f;

        return fmt::format("{}.{}.{}.{}", major, minor, secondary, tertiary);
    }
}

namespace Engine
{
    // ----- Public -----

    VulkanPhysicalDevice::VulkanPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
    {
        PickDevice(instance, surface);
        PrintDeviceSpecifics();
    }

    void VulkanPhysicalDevice::PickDevice(vk::Instance instance, vk::SurfaceKHR surface)
    {
        // Query for devices
        auto devices = instance.enumeratePhysicalDevices();
        ASSERT(!devices.empty(), "Failed to find GPUs with Vulkan support!");

        LOG_INFO("Check for suitable device ...");

        // Check each device for suitability
        for(const auto& device : devices)
        {
            // Break at first suitable device
            if(IsDeviceSuitable(device, surface))
            {
                m_PhysicalDevice = device;
                m_Properties = device.getProperties();
                LOG_INFO("Found suitable device ...");
                break;
            }
        }

        ASSERT(m_PhysicalDevice, "Failed to find suitable device!");
    }

    // ----- Private -----

    bool VulkanPhysicalDevice::IsDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        m_queueFamilyIndices = FindQueueFamilyIndices(device, surface);
        return m_queueFamilyIndices.isComplete();
    }

    QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilyIndices(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        QueueFamilyIndices queueFamilyIndices;
        VkBool32 presentSupport = false;
        i32 index = 0;

        auto queueFamilies = device.getQueueFamilyProperties();

        // Iterate over queue familys
        for(const auto& queueFamily : queueFamilies)
        {
            // Query for graphics capable queue family
            if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                // Query for capability of presenting to a window surface
                VK_VERIFY((vk::Result)device.getSurfaceSupportKHR(index, surface, &presentSupport));

                if(presentSupport)
                {
                    queueFamilyIndices.GraphicsFamily = index;
                }
            }

            index++;
        }

        return queueFamilyIndices;
    }

    void VulkanPhysicalDevice::PrintDeviceSpecifics()
    {
        LOG_INFO("GPU: {}, Driver: {}", (const char*)m_Properties.deviceName,
                 GetDriverVersionString(m_Properties));
    }
}
