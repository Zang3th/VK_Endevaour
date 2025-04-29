#include "VulkanContext.hpp"
#include "VulkanDefines.hpp"
#include "VulkanAssert.hpp"
#include "VulkanDebug.hpp"
#include "Core/Window.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_core.h>

namespace
{
    // ----- Internal -----

    const std::vector<const char*> s_ValidationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };
}

namespace Engine
{
    // ----- Public -----

    VulkanContext::VulkanContext()
    {
        CreateInstance();

        if(ENABLE_VALIDATION_LAYERS)
        {
            CreateDebugMessenger();
        }
    }

    VulkanContext::~VulkanContext()
    {
        if(ENABLE_VALIDATION_LAYERS)
        {
            m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger);
        }

        m_Instance.destroy();
    }

    // ----- Private -----

    void VulkanContext::CreateInstance()
    {
        if(ENABLE_VALIDATION_LAYERS)
        {
            ASSERT(VulkanDebug::CheckValidationLayerSupport(s_ValidationLayers),
                   "Validation layers were requested, but not available!");
        }
        const std::string& title = Window::GetTitle();

        vk::ApplicationInfo appInfo
        {
            .pApplicationName   = title.c_str(),
            .applicationVersion = 1,
            .pEngineName        = "Engine",
            .engineVersion      = 1,
            .apiVersion         = VK_API_VERSION_1_4
        };

        auto extensions = VulkanDebug::GetExtensions();

        // Create temporary debug messenger to trace instantiating
        auto debugCreateInfo = VulkanDebug::GetDebugCreateInfo();

        vk::InstanceCreateInfo createInfo
        {
            .pNext                   = ENABLE_VALIDATION_LAYERS ? &debugCreateInfo : nullptr,
            .pApplicationInfo        = &appInfo,
            .enabledLayerCount       = ENABLE_VALIDATION_LAYERS ? (u32)(s_ValidationLayers.size()) : 0,
            .ppEnabledLayerNames     = ENABLE_VALIDATION_LAYERS ? s_ValidationLayers.data() : nullptr,
            .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VK_VERIFY(vk::createInstance(&createInfo, nullptr, &m_Instance));
        LOG_INFO("Created Vulkan instance ...");
    }

    void VulkanContext::CreateDebugMessenger()
    {
        VulkanDebug::LoadDebugExtensionFunctions(m_Instance);
        m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(VulkanDebug::GetDebugCreateInfo());
        ASSERT(m_DebugMessenger , "Failed to create debug messenger!");
        LOG_INFO("Created debug messenger ...");
    }
}
