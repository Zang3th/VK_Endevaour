#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine
{
    struct QueueFamilyIndices
    {
        i32 GraphicsFamily = -1;

        [[nodiscard]] b8 isComplete() const
        {
            return (GraphicsFamily >= 0);
        };
    };

    struct SwapchainSupport
    {
        vk::SurfaceCapabilitiesKHR        Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR>   PresentModes;

        [[nodiscard]] b8 isComplete() const
        {
            return (!Formats.empty() && !PresentModes.empty());
        };
    };

    class VulkanPhysicalDevice
    {
        public:
            VulkanPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

            [[nodiscard]] const vk::PhysicalDevice& GetHandle()           const { return m_PhysicalDevice;     };
            [[nodiscard]] const QueueFamilyIndices& GetQueueFamilys()     const { return m_QueueFamilyIndices; };
            [[nodiscard]] const SwapchainSupport&   GetSwapchainSupport() const { return m_SwapchainSupport;   };
            [[nodiscard]] const vk::PhysicalDeviceProperties& GetProperties() const { return m_Properties;     };

        private:
            void               PickDevice(vk::Instance instance, vk::SurfaceKHR surface);
            bool               IsDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);
            QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice device, vk::SurfaceKHR surface);
            SwapchainSupport   QuerySwapchainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
            bool               CheckDeviceExtensionSupport(vk::PhysicalDevice device);

            vk::PhysicalDevice           m_PhysicalDevice;
            QueueFamilyIndices           m_QueueFamilyIndices;
            SwapchainSupport             m_SwapchainSupport;
            vk::PhysicalDeviceProperties m_Properties;
    };
}
