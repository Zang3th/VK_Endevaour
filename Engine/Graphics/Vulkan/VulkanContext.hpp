#pragma once

#include "Core/Memory.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanDevice.hpp"

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"

namespace Engine
{
    class VulkanContext
    {
        public:
            VulkanContext();
            ~VulkanContext();

            [[nodiscard]] const vk::Instance& GetInstance() const { return m_Instance; }

        private:
            void CreateInstance();
            void CreateDebugMessenger();
            void CreatePhysicalDevice();

            vk::Instance                m_Instance;
            vk::DebugUtilsMessengerEXT  m_DebugMessenger;
            Scope<VulkanPhysicalDevice> m_PhysicalDevice;
            Scope<VulkanDevice>         m_Device;
    };
}
