#include "VulkanContext.hpp"

#include "Core/Utility.hpp"

#include "Graphics/Vulkan/VulkanAllocator.hpp"
#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanDebug.hpp"
#include "Graphics/Vulkan/VulkanDevice.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"
#include "Graphics/Vulkan/VulkanPhysicalDevice.hpp"
#include "Graphics/Vulkan/VulkanSwapchain.hpp"

#include "Platform/Window.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanContext::VulkanContext(const Platform::Window& window)
    {
        CreateInstance(window);
        window.CreateVulkanSurface(m_Instance, &m_Surface);

        if (ENABLE_VALIDATION_LAYERS)
        {
            CreateDebugMessenger();
        }

        m_PhysicalDevice = MakeScope<VulkanPhysicalDevice>(m_Instance, m_Surface);
        m_Device         = MakeScope<VulkanDevice>(m_PhysicalDevice.get());

        VulkanAllocator::Init(m_Device.get(), m_Instance, m_PhysicalDevice->GetApiVersion());
        m_Swapchain = MakeScope<VulkanSwapchain>(m_Device.get(), m_Surface, window);

        CreateDispatchLoader(); // For later use in extension functions
    }

    VulkanContext::~VulkanContext()
    {
        m_Swapchain.reset();
        VulkanAllocator::Shutdown();

        m_Device.reset();
        m_PhysicalDevice.reset();

        if (ENABLE_VALIDATION_LAYERS)
        {
            m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr);
            LOG_INFO("Destroyed debug messenger ...");
        }

        m_Instance.destroySurfaceKHR(m_Surface);
        LOG_INFO("Destroyed Vulkan surface ...");

        m_Instance.destroy();
        LOG_INFO("Destroyed Vulkan instance ...");
    }

    void VulkanContext::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        const vk::CommandBuffer commandBuffer = m_Swapchain->CreateTransferCommandBuffer();
        const vk::BufferCopy    bufferCopy    = { .size = size };
        commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &bufferCopy);
        m_Swapchain->SubmitTransferCommandBuffer(commandBuffer);
        LOG_MEM("Copied {} from CPU => GPU ...", Core::Utility::BytesToString(size));
    }

    // ----- Private -----

    void VulkanContext::CreateInstance(const Platform::Window& window)
    {
        if (ENABLE_VALIDATION_LAYERS)
        {
            ASSERT(VulkanDebug::CheckValidationLayerSupport(g_ValidationLayers),
                   "Validation layers were requested, but not available!");
        }

        const vk::ApplicationInfo appInfo{ .pApplicationName   = window.GetTitle().c_str(),
                                           .applicationVersion = 1,
                                           .pEngineName        = "Engine",
                                           .engineVersion      = 1,
                                           .apiVersion         = VULKAN_API_TARGET_VERSION };

        // Query for all required instance extensions
        auto extensions = VulkanDebug::GetInstanceExtensions(window);

        // Create temporary debug messenger to trace instantiating
        auto debugCreateInfo = VulkanDebug::GetDebugCreateInfo();

        const vk::InstanceCreateInfo createInfo{
            .pNext                   = ENABLE_VALIDATION_LAYERS ? &debugCreateInfo : nullptr,
            .pApplicationInfo        = &appInfo,
            .enabledLayerCount       = ENABLE_VALIDATION_LAYERS ? (u32)g_ValidationLayers.size() : 0,
            .ppEnabledLayerNames     = ENABLE_VALIDATION_LAYERS ? g_ValidationLayers.data() : nullptr,
            .enabledExtensionCount   = (u32)extensions.size(),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VK_VERIFY(vk::createInstance(&createInfo, nullptr, &m_Instance));
        LOG_INFO("Created Vulkan instance ...");
    }

    void VulkanContext::CreateDebugMessenger()
    {
        VulkanDebug::LoadDebugExtensionFunctions(m_Instance);
        const auto [result, debugMessenger] =
            m_Instance.createDebugUtilsMessengerEXT(VulkanDebug::GetDebugCreateInfo(), nullptr);
        ASSERT(debugMessenger, "Failed to create debug messenger!");
        m_DebugMessenger = debugMessenger;
        LOG_INFO("Created debug messenger ...");
    }

    void VulkanContext::CreateDispatchLoader()
    {
        // vkGetInstanceProcAddr
        auto pfnVkGetInstanceProcAddr =
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(m_Instance.getProcAddr("vkGetInstanceProcAddr"));
        ASSERT(pfnVkGetInstanceProcAddr, "Unable to load vkGetInstanceProcAddr");

        // vkGetDeviceProcAddr
        auto pfnVkGetDeviceProcAddr =
            reinterpret_cast<PFN_vkGetDeviceProcAddr>(m_Instance.getProcAddr("vkGetDeviceProcAddr"));
        ASSERT(pfnVkGetDeviceProcAddr, "Unable to load vkGetDeviceProcAddr");

        // Init loader
        m_DispatchLoader = vk::detail::DispatchLoaderDynamic(
            m_Instance, pfnVkGetInstanceProcAddr, m_Device->GetHandle(), pfnVkGetDeviceProcAddr);
        LOG_INFO("Created dynamic dispatch loader ...");
    }
}
