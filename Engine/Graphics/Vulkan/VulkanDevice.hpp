#pragma once

#include "VulkanPhysicalDevice.hpp"

namespace Engine
{
    class VulkanDevice
    {
        public:
            explicit VulkanDevice(const VulkanPhysicalDevice* physicalDevice);
            ~VulkanDevice();

            [[nodiscard]] const vk::Device&            GetHandle()         const { return m_Device;         }
            [[nodiscard]] const vk::Queue&             GetGraphicsQueue()  const { return m_GraphicsQueue;  }
            [[nodiscard]] const VulkanPhysicalDevice*  GetPhysicalDevice() const { return m_PhysicalDevice; }

        private:
            void CreateLogicalDevice();

            vk::Device                  m_Device;
            vk::Queue                   m_GraphicsQueue;
            const VulkanPhysicalDevice* m_PhysicalDevice = nullptr;
    };
}
