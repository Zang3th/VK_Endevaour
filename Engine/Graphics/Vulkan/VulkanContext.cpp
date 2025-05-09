#include "VulkanContext.hpp"
#include "VulkanGlobals.hpp"
#include "VulkanAssert.hpp"
#include "VulkanDebug.hpp"
#include "VulkanAllocator.hpp"

#include "Core/Memory.hpp"
#include "Core/Window.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_core.h>

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

        CreateSurface();
        m_PhysicalDevice = MakeScope<VulkanPhysicalDevice>(m_Instance, m_Surface);
        m_Device         = MakeScope<VulkanDevice>(m_PhysicalDevice.get());

        VulkanAllocator::Init(m_PhysicalDevice->GetHandle(), m_Device->GetHandle(), m_Instance);

        // Initialize and create swapchain
        m_Swapchain = MakeScope<VulkanSwapchain>(m_Device.get(), &m_Surface);
        m_Swapchain->Create();
    }

    VulkanContext::~VulkanContext()
    {
        m_Swapchain->Destroy();

        VulkanAllocator::Shutdown();

        m_Instance.destroySurfaceKHR(m_Surface);

        if(ENABLE_VALIDATION_LAYERS)
        {
            m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger);
        }

        m_Instance.destroy();
    }

    void VulkanContext::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        // TODO: Implement
    }

    // ----- Private -----

    void VulkanContext::CreateInstance()
    {
        if(ENABLE_VALIDATION_LAYERS)
        {
            ASSERT(VulkanDebug::CheckValidationLayerSupport(g_ValidationLayers),
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
            .enabledLayerCount       = ENABLE_VALIDATION_LAYERS ? (u32)(g_ValidationLayers.size()) : 0,
            .ppEnabledLayerNames     = ENABLE_VALIDATION_LAYERS ? g_ValidationLayers.data() : nullptr,
            .enabledExtensionCount   = (u32)(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VK_VERIFY(vk::createInstance(&createInfo, nullptr, &m_Instance));
        LOG_INFO("Created Vulkan instance ...");
    }

    void VulkanContext::CreateSurface()
    {
        VK_VERIFY((vk::Result)(glfwCreateWindowSurface(m_Instance, Window::GetHandle(), nullptr,
                                                      (VkSurfaceKHR*)&m_Surface)));
        LOG_INFO("Created window surface ...");
    }

    void VulkanContext::CreateDebugMessenger()
    {
        VulkanDebug::LoadDebugExtensionFunctions(m_Instance);
        const auto [result, m_DebugMessenger] = m_Instance.createDebugUtilsMessengerEXT(VulkanDebug::GetDebugCreateInfo());
        ASSERT(m_DebugMessenger , "Failed to create debug messenger!");
        LOG_INFO("Created debug messenger ...");
    }
}
