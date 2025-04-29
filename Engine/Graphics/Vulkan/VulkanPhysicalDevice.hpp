#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

#include <optional>

namespace Engine
{
    struct QueueFamilyIndices
    {
        std::optional<u32> GraphicsFamily;

        [[nodiscard]] bool isComplete() const
        {
            return GraphicsFamily.has_value();
        };
    };

    class VulkanPhysicalDevice
    {
        public:
            VulkanPhysicalDevice(vk::Instance instance);

            [[nodiscard]] vk::PhysicalDevice GetHandle() const { return m_PhysicalDevice; };

        private:
            void PickDevice(vk::Instance instance);
            bool IsDeviceSuitable(vk::PhysicalDevice device);
            QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice device);
            void PrintDeviceSpecifics(vk::PhysicalDevice device);

            vk::PhysicalDevice m_PhysicalDevice;
            QueueFamilyIndices m_queueFamilyIndices;
    };
}
