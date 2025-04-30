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

    class VulkanPhysicalDevice
    {
        public:
            VulkanPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

            [[nodiscard]] const vk::PhysicalDevice&           GetHandle()     const { return m_PhysicalDevice; };
            [[nodiscard]] const QueueFamilyIndices&           GetIndices()    const { return m_queueFamilyIndices; };
            [[nodiscard]] const vk::PhysicalDeviceProperties& GetProperties() const { return m_Properties; };

        private:
            void PickDevice(vk::Instance instance, vk::SurfaceKHR surface);
            bool IsDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);
            QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice device, vk::SurfaceKHR surface);
            void PrintDeviceSpecifics();

            vk::PhysicalDevice m_PhysicalDevice;
            QueueFamilyIndices m_queueFamilyIndices;
            vk::PhysicalDeviceProperties m_Properties;
    };
}
