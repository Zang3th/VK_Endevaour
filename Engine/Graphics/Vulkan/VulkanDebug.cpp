#include "VulkanDebug.hpp"
#include "VulkanGlobals.hpp"

#include "Debug/Log.hpp"
#include "Platform/Window.hpp"

// Define two function pointers in anonymous namespace. They will later point to Vulkan extension functions
namespace
{
    PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT  = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT = nullptr;
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
    pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

namespace Engine
{
    // ----- Public -----

    bool VulkanDebug::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
    {
        u32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for(const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for(const auto& layerProperties : availableLayers)
            {
                if(strcmp(layerName, layerProperties.layerName) == 0)
                {
                    LOG_INFO("Found requested validation layer ... ({})", layerName);
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound)
            {
                LOG_ERROR("Validation layer is missing: {}", layerName);
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> VulkanDebug::GetInstanceExtensions()
    {
        u32 glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if(ENABLE_VALIDATION_LAYERS)
        {
            // Add the debug messenger extension conditionally
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            // Log extensions
            for(const auto& ext : extensions)
            {
                LOG_INFO("Application requires instance extension: {} ...", ext);
            }
        }

        return extensions;
    }

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
                LOG_VERBOSE("{} {}", vk::to_string(messageType), pCallbackData->pMessage);
                break;

            case Severity::eInfo:
                LOG_INFO("{} {}", vk::to_string(messageType), pCallbackData->pMessage);
                break;

            case Severity::eWarning:
                LOG_WARN("{} {}", vk::to_string(messageType), pCallbackData->pMessage);
                break;

            case Severity::eError:
                LOG_ERROR("##################################");
                LOG_ERROR("##### VulkanDebug::Callback ######");
                LOG_ERROR("##################################");
                LOG_ERROR("{} {}", vk::to_string(messageType), pCallbackData->pMessage);
                ASSERT(false, "Caught an error with VulkanDebug::Callback!");
                break;

            default:
                LOG_ERROR("Caught an unknown severity in VulkanDebug::Callback!");
                break;
        }

        return VK_FALSE;
    }

    vk::DebugUtilsMessengerCreateInfoEXT VulkanDebug::GetDebugCreateInfo()
    {
        return
        {
            .messageSeverity =
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
    void VulkanDebug::LoadDebugExtensionFunctions(vk::Instance instance)
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

        LOG_INFO("Loaded debug extension functions ...");
    }
}
