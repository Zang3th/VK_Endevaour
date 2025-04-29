#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"

namespace Engine
{
    class VulkanContext
    {
        public:
            VulkanContext();
            ~VulkanContext();

            [[nodiscard]] vk::Instance GetInstance() { return m_Instance; }

        private:
            void CreateInstance();
            void CreateDebugMessenger();

            vk::Instance               m_Instance;
            vk::DebugUtilsMessengerEXT m_DebugMessenger;
    };
}
