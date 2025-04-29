#pragma once

#include "VulkanPhysicalDevice.hpp"

namespace Engine
{
    class VulkanDevice
    {
        public:
            explicit VulkanDevice(const VulkanPhysicalDevice& physicalDevice);
            ~VulkanDevice();

            VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }

        private:
            void CreateLogicalDevice(const VulkanPhysicalDevice& physicalDevice);

            vk::Device m_Device;
            vk::Queue  m_GraphicsQueue;
    };
}
