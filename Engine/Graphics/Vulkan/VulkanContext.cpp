#ifdef NDEBUG
    const bool ENABLE_VALIDATION_LAYERS = false;
#else
    const bool ENABLE_VALIDATION_LAYERS = true;
#endif

#include "VulkanContext.hpp"
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

    bool CheckValidationLayerSupport()
    {
        LOG_INFO("Check for all requested validation layers ...");

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for(const char* layerName : s_ValidationLayers)
        {
            bool layerFound = false;

            for(const auto& layerProperties : availableLayers)
            {
                if(strcmp(layerName, layerProperties.layerName) == 0)
                {
                    LOG_INFO("Found layer: {} ...", layerName);
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

    std::vector<const char*> GetGlfwExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // Add the debug messenger extension conditionally
        if(ENABLE_VALIDATION_LAYERS)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }
}

namespace Engine
{
    // ----- Public -----

    VulkanContext::VulkanContext()
    {
        CreateInstance();
        SetupDebugMessenger();
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
            ASSERT(CheckValidationLayerSupport(), "Validation layers were requested, but not available!");
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

        auto extensions      = GetGlfwExtensions();

        // Create temporary debug messenger to trace instantiating
        auto debugCreateInfo = VulkanDebug::GetCreateInfo();

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

    void VulkanContext::SetupDebugMessenger()
    {
        if(ENABLE_VALIDATION_LAYERS)
        {
            VulkanDebug::LoadExtensionFunctions(m_Instance);
            auto debugCreateInfo = VulkanDebug::GetCreateInfo();
            m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(debugCreateInfo);
            ASSERT(m_DebugMessenger, "Failed to create debug messenger!");
            LOG_INFO("Created debug messenger ...");
        }

        // --- DO SOME CRAP TO TEST VALIDATION LAYER ERROR HANDLING --- //

        // --- Physical Device ---
        vk::PhysicalDevice physicalDevice = m_Instance.enumeratePhysicalDevices().front();

        // --- Device & Queue ---
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueInfo{
            .queueFamilyIndex = 0,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        vk::DeviceCreateInfo deviceInfo{
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueInfo,
            .enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size()),
            .ppEnabledLayerNames = s_ValidationLayers.data()
        };

        vk::Device device = physicalDevice.createDevice(deviceInfo);

        vk::CommandPoolCreateInfo poolInfo{ .queueFamilyIndex = 0 };
        vk::CommandPool cmdPool = device.createCommandPool(poolInfo);

        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool = cmdPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };

        vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers(allocInfo).front();

        // --- Begin + Draw without pipeline ---
        vk::CommandBufferBeginInfo beginInfo{};
        cmdBuffer.begin(beginInfo);

        // ❌ No pipeline bound → guaranteed validation error
        cmdBuffer.draw(3, 1, 0, 0);

        cmdBuffer.end();
    }
}
