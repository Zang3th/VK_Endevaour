#pragma once

#include "Core/Types.hpp"

#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    struct QueueFamilyIndices
    {
        u32 GraphicsFamily = UINT32_MAX;
        u32 TransferFamily = UINT32_MAX;

        [[nodiscard]] b8 HasGraphicsFamily() const { return GraphicsFamily != UINT32_MAX; }
        [[nodiscard]] b8 HasTransferFamily() const { return TransferFamily != UINT32_MAX; }
        [[nodiscard]] b8 IsComplete() const
        {
            return ((GraphicsFamily != UINT32_MAX) && (TransferFamily != UINT32_MAX));
        };
    };

    class VulkanPhysicalDevice
    {
    public:
        VulkanPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface);
        ~VulkanPhysicalDevice();

        [[nodiscard]] const vk::PhysicalDevice&           GetHandle() const { return m_PhysicalDevice; };
        [[nodiscard]] const QueueFamilyIndices&           GetQueueFamilies() const { return m_QueueFamilyIndices; };
        [[nodiscard]] const vk::PhysicalDeviceProperties& GetProperties() const { return m_Properties; };

        [[nodiscard]] SwapchainSupport GetSwapchainSupport() const { return QuerySwapchainSupport(m_PhysicalDevice); };

    private:
        void        PickDevice();
        static bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);

        [[nodiscard]] bool               IsDeviceSuitable(vk::PhysicalDevice device) const;
        [[nodiscard]] QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice device) const;
        [[nodiscard]] SwapchainSupport   QuerySwapchainSupport(vk::PhysicalDevice physicalDevice) const;

        const vk::Instance&          m_Instance;
        const vk::SurfaceKHR&        m_Surface;
        vk::PhysicalDevice           m_PhysicalDevice;
        QueueFamilyIndices           m_QueueFamilyIndices;
        vk::PhysicalDeviceProperties m_Properties;
    };
}
