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
        LOG_INFO("VulkanDevice::Destructor() ...");
        m_Device.destroy();
    }

    void VulkanDevice::WaitForIdle() const
    {
        VK_VERIFY(m_Device.waitIdle());
    }

    // ----- Private -----

    void VulkanDevice::CreateLogicalDevice()
    {
        // Fetch queue familys
        const auto& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilys();
        static const f32 queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        // Add queue infos for graphics
        queueCreateInfos.push_back
        ({
            .queueFamilyIndex = (u32)queueFamilyIndices.GraphicsFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        });

        // If transfer uses a different queue, add it too
        if(queueFamilyIndices.GraphicsFamily != queueFamilyIndices.TransferFamily)
        {
            queueCreateInfos.push_back
            ({
                .queueFamilyIndex = (u32)queueFamilyIndices.TransferFamily,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority,
            });
        }

        // Define features you want to use (e.g. geometry shaders)
        vk::PhysicalDeviceFeatures deviceFeatures{};

        // Activate dynamic rendering and synchronization2
        vk::PhysicalDeviceVulkan13Features vulkan13Features
        {
            .pNext            = nullptr,
            .synchronization2 = vk::True,
            .dynamicRendering = vk::True
        };

        // Activate dynamic state 3 extension
        vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT extDyn3Features
        {
            .pNext                            = &vulkan13Features,
            .extendedDynamicState3PolygonMode = vk::True
        };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
        // Configure logical device
        vk::DeviceCreateInfo deviceCreateInfo
        {
            .pNext                   = &extDyn3Features,
            .queueCreateInfoCount    = (u32)(queueCreateInfos.size()),
            .pQueueCreateInfos       = queueCreateInfos.data(),
            .enabledExtensionCount   = (u32)(g_DeviceExtensions.size()),
            .ppEnabledExtensionNames = g_DeviceExtensions.data(),
            .pEnabledFeatures        = &deviceFeatures
        };
#pragma clang diagnostic pop

        // Create logical device
        VK_VERIFY(m_PhysicalDevice->GetHandle().createDevice(&deviceCreateInfo, nullptr, &m_Device));
        LOG_INFO("Created logical device ...");

        // Retrieve queue handles
        m_Device.getQueue(queueFamilyIndices.GraphicsFamily, 0, &m_GraphicsQueue);
        m_Device.getQueue(queueFamilyIndices.TransferFamily, 0, &m_TransferQueue);

        LOG_INFO("Retrieved queue family handles ... (Graphics: {}, Transfer: {})", queueFamilyIndices.GraphicsFamily, queueFamilyIndices.TransferFamily);
    }
}
