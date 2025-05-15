#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine
{
    struct QueueFamilyIndices
    {
        u32 GraphicsFamily = UINT32_MAX;
        u32 TransferFamily = UINT32_MAX;

        [[nodiscard]] b8 isComplete() const
        {
            return ((GraphicsFamily != UINT32_MAX) && (TransferFamily != UINT32_MAX));
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
            VulkanPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface);

            [[nodiscard]] const vk::PhysicalDevice& GetHandle()           const { return m_PhysicalDevice;     };
            [[nodiscard]] const QueueFamilyIndices& GetQueueFamilys()     const { return m_QueueFamilyIndices; };
            [[nodiscard]] const vk::PhysicalDeviceProperties& GetProperties() const { return m_Properties;     };

            [[nodiscard]] SwapchainSupport GetSwapchainSupport() const
            {
                return QuerySwapchainSupport(m_PhysicalDevice);
            };

        private:
            void               PickDevice();
            bool               IsDeviceSuitable(vk::PhysicalDevice device);
            QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice device);
            bool               CheckDeviceExtensionSupport(vk::PhysicalDevice device);

            [[nodiscard]] SwapchainSupport QuerySwapchainSupport(vk::PhysicalDevice physicalDevice) const;

            const vk::Instance&          m_Instance;
            const vk::SurfaceKHR&        m_Surface;
            vk::PhysicalDevice           m_PhysicalDevice;
            QueueFamilyIndices           m_QueueFamilyIndices;
            vk::PhysicalDeviceProperties m_Properties;
    };
}
