#pragma once

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    class VulkanDebug
    {
    public:
        ~VulkanDebug() = delete;

        static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

        static void                     LoadDebugExtensionFunctions(vk::Instance instance);
        static std::vector<const char*> GetInstanceExtensions();

        // clang-format off
        static VKAPI_ATTR VkBool32 VKAPI_CALL Callback
        (
            vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
            vk::DebugUtilsMessageTypeFlagsEXT             messageType,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*                                         pUserData
        );
        // clang-format on

        static vk::DebugUtilsMessengerCreateInfoEXT GetDebugCreateInfo();
    };
}
