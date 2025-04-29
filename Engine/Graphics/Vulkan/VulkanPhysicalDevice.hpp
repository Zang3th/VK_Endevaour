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
            explicit VulkanPhysicalDevice(vk::Instance instance);

            [[nodiscard]] const vk::PhysicalDevice&           GetHandle()     const { return m_PhysicalDevice; };
            [[nodiscard]] const QueueFamilyIndices&           GetIndices()    const { return m_queueFamilyIndices; };
            [[nodiscard]] const vk::PhysicalDeviceProperties& GetProperties() const { return m_Properties; };

        private:
            void PickDevice(vk::Instance instance);
            bool IsDeviceSuitable(vk::PhysicalDevice device);
            QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice device);
            void PrintDeviceSpecifics();

            vk::PhysicalDevice m_PhysicalDevice;
            QueueFamilyIndices m_queueFamilyIndices;
            vk::PhysicalDeviceProperties m_Properties;
    };
}
