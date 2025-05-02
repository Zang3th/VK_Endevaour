#include "VulkanSwapchain.hpp"
#include "VulkanAssert.hpp"

namespace Engine
{
    // ----- Public -----

    void VulkanSwapchain::Init(const VulkanDevice* device, const vk::SurfaceKHR* surface)
    {
        m_Device         = device;
        m_Surface        = surface;

        QuerySwapchainSupport(m_Device->GetPhysicalDevice()->GetHandle());
    }

    void VulkanSwapchain::Create(u32 width, u32 height)
    {

    }

    void VulkanSwapchain::Recreate(u32 width, u32 height)
    {

    }

    void VulkanSwapchain::Destroy()
    {

    }

    // ----- Private -----

    void VulkanSwapchain::QuerySwapchainSupport(vk::PhysicalDevice physicalDevice)
    {
        // m_Support.Capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*m_Surface);
        // m_Support.Formats      = physicalDevice.getSurfaceFormatsKHR(*m_Surface);
        // m_Support.PresentModes = physicalDevice.getSurfacePresentModesKHR(*m_Surface);

        // auto [result, formats] = physicalDevice.getSurfaceFormatsKHR(*m_Surface);
        // VK_VERIFY(result);
        // m_Support.Formats = std::move(formats);

        // TODO: Enable structured bindings
    }
}
