#include "VulkanDevice.hpp"
#include "VulkanGlobals.hpp"
#include "VulkanAssert.hpp"

#include <vulkan/vulkan_structs.hpp>

namespace Engine
{
    // ----- Public -----

    VulkanDevice::VulkanDevice(const VulkanPhysicalDevice* physicalDevice)
    {
        m_PhysicalDevice = physicalDevice;
        CreateLogicalDevice();
    }

    VulkanDevice::~VulkanDevice()
    {
        m_Device.destroy();
    }

    void VulkanDevice::CreateLogicalDevice()
    {
        // Add graphics queue
        const auto& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilys();
        static const float queuePriority = 1.0f;

        vk::DeviceQueueCreateInfo queueCreateInfo
        {
            .queueFamilyIndex = (u32)queueFamilyIndices.GraphicsFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };

        // Define features you want to use (e.g. geometry shaders)
        vk::PhysicalDeviceFeatures deviceFeatures{};

        // Configure logical device
        vk::DeviceCreateInfo deviceCreateInfo
        {
            .queueCreateInfoCount    = 1,
            .pQueueCreateInfos       = &queueCreateInfo,
            .enabledExtensionCount   = (u32)(g_DeviceExtensions.size()),
            .ppEnabledExtensionNames = g_DeviceExtensions.data(),
            .pEnabledFeatures        = &deviceFeatures,
        };

        // Create logical device
        VK_VERIFY(m_PhysicalDevice->GetHandle().createDevice(&deviceCreateInfo, nullptr, &m_Device));
        LOG_INFO("Created logical device ...");

        // Retrieve graphics queue handle
        m_Device.getQueue(queueFamilyIndices.GraphicsFamily, 0, &m_GraphicsQueue);
    }
}
