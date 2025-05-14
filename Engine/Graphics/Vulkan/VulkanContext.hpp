#pragma once

#include "VulkanPhysicalDevice.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

#include "Core/Memory.hpp"

#include <vulkan/vulkan_handles.hpp>

namespace Engine
{
    class VulkanContext
    {
        public:
            VulkanContext();
            ~VulkanContext();

            [[nodiscard]] const vk::Instance&    GetInstance()  const { return m_Instance;        }
            [[nodiscard]] const vk::SurfaceKHR&  GetSurface()   const { return m_Surface;         }
            [[nodiscard]] const VulkanDevice*    GetDevice()    const { return m_Device.get();    }
            [[nodiscard]] const VulkanSwapchain* GetSwapchain() const { return m_Swapchain.get(); }

            [[nodiscard]] const vk::detail::DispatchLoaderDynamic& GetLoader() const { return m_DispatchLoader; }

            void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

        private:
            void CreateInstance();
            void CreateDebugMessenger();
            void CreateSurface();
            void CreateDispatchLoader();

            vk::Instance                m_Instance;
            vk::SurfaceKHR              m_Surface;
            vk::DebugUtilsMessengerEXT  m_DebugMessenger;
            Scope<VulkanPhysicalDevice> m_PhysicalDevice;
            Scope<VulkanDevice>         m_Device;
            Scope<VulkanSwapchain>      m_Swapchain;

            vk::detail::DispatchLoaderDynamic m_DispatchLoader;
    };
}
