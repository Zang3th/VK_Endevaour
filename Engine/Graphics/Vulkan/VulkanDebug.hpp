#pragma once

#include "Debug/Log.hpp"

#include "vulkan/vulkan.hpp"

namespace Engine
{
    #define VK_VERIFY(func) \
    { \
        vk::Result res = (func); \
        if(res != vk::Result::eSuccess) \
        { \
            LOG_ERROR("[VkResult] {} in\n\t\t\t{}:{}", vk::to_string(res), __FILE__, __LINE__); \
            __builtin_trap(); \
        } \
    }

    class VulkanDebug
    {
        public:
            ~VulkanDebug() = delete;

            static VKAPI_ATTR VkBool32 VKAPI_CALL
            Callback
            (
                vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                vk::DebugUtilsMessageTypeFlagsEXT             messageType,
                const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*                                         pUserData
            );

            static vk::DebugUtilsMessengerCreateInfoEXT GetCreateInfo();
            static void LoadExtensionFunctions(vk::Instance instance);
    };
}
