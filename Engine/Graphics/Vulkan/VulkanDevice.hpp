#pragma once

#include "VulkanPhysicalDevice.hpp"

namespace Engine
{
    class VulkanDevice
    {
        public:
            explicit VulkanDevice(const VulkanPhysicalDevice* physicalDevice);
            ~VulkanDevice();

            [[nodiscard]] const vk::Device&           GetHandle()         const { return m_Device;         }
            [[nodiscard]] const vk::Queue&            GetGraphicsQueue()  const { return m_GraphicsQueue;  }
            [[nodiscard]] const vk::Queue&            GetTransferQueue()  const { return m_TransferQueue;  }
            [[nodiscard]] const VulkanPhysicalDevice* GetPhysicalDevice() const { return m_PhysicalDevice; }

            [[nodiscard]] u32 GetGraphicsQueueFamily() const { return m_PhysicalDevice->GetQueueFamilys().GraphicsFamily; }
            [[nodiscard]] u32 GetTransferQueueFamily() const { return m_PhysicalDevice->GetQueueFamilys().TransferFamily; }

        private:
            void CreateLogicalDevice();

            vk::Device                  m_Device;
            vk::Queue                   m_GraphicsQueue;
            vk::Queue                   m_TransferQueue;
            const VulkanPhysicalDevice* m_PhysicalDevice = nullptr;
    };
}
