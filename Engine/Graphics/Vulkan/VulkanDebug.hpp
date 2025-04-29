#pragma once

#include "vulkan/vulkan.hpp"

namespace Engine
{
    class VulkanDebug
    {
        public:
            ~VulkanDebug() = delete;

            static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
            static std::vector<const char*> GetExtensions();
            static VKAPI_ATTR VkBool32 VKAPI_CALL Callback
            (
                vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                vk::DebugUtilsMessageTypeFlagsEXT             messageType,
                const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*                                         pUserData
            );
            static vk::DebugUtilsMessengerCreateInfoEXT GetDebugCreateInfo();
            static void LoadDebugExtensionFunctions(vk::Instance instance);
    };
}
