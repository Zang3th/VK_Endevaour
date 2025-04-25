#include "VulkanDebug.hpp"

// Define two function pointers in anonymous namespace
// They will later point to Vulkan extension functions
namespace
{
    PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;
}

// Forward global Vulkan function calls to dynamically loaded extension functions
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT
(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pMessenger
)
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT
(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     messenger,
    VkAllocationCallbacks const* pAllocator
)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

namespace Engine
{
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebug::Callback
    (
        vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT             messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*                                         pUserData
    )
    {
        using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;

        switch (messageSeverity)
        {
            case Severity::eVerbose:
                LOG_VERBOSE("[VkMsg | {}]\n{}", vk::to_string(messageType), pCallbackData->pMessage);
                break;

            case Severity::eInfo:
                LOG_INFO("[VkMsg | {}]\n{}", vk::to_string(messageType), pCallbackData->pMessage);
                break;

            case Severity::eWarning:
                LOG_WARN("[VkMsg | {}]\n{}", vk::to_string(messageType), pCallbackData->pMessage);
                break;

            case Severity::eError:
                LOG_ERROR("[VkMsg | {}]\n{}", vk::to_string(messageType), pCallbackData->pMessage);
                ASSERT(false, "VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT");
                break;

            default:
                LOG_ERROR("Caught unknown severity in VulkanDebug::Callback!");
                break;
        }

        return VK_FALSE;
    }

    vk::DebugUtilsMessengerCreateInfoEXT VulkanDebug::GetCreateInfo()
    {
        return
        {
            .messageSeverity =
                // vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral     |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation  |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = VulkanDebug::Callback,
            .pUserData = nullptr
        };
    }

    // Load and assign Vulkan extension functions
    void VulkanDebug::LoadExtensionFunctions(vk::Instance instance)
    {
        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
        (
            instance.getProcAddr("vkCreateDebugUtilsMessengerEXT")
        );
        ASSERT(pfnVkCreateDebugUtilsMessengerEXT, "Unable to load vkCreateDebugUtilsMessengerEXT");

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
        (
            instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT")
        );
        ASSERT(pfnVkDestroyDebugUtilsMessengerEXT, "Unable to load vkDestroyDebugUtilsMessengerEXT");
    }
}
