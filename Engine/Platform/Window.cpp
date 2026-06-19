#include "Window.hpp"

#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"

#include "Vendor/glfw/include/GLFW/glfw3.h"

namespace Engine::Platform
{
    // ----- Public -----

    void Window::Init(const WindowSpecification& spec)
    {
        m_Spec = spec;

        ASSERT(glfwSetErrorCallback(&GLFW_ErrorCallback) == nullptr, "GLFW::ErrorCallback was already set!");
        ASSERT(glfwInit(), "Failed to initialize GLFW!");
        LOG_INFO("Initialized GLFW ...");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow((int)m_Spec.Width, (int)m_Spec.Height, m_Spec.Title.c_str(), nullptr, nullptr);
        ASSERT(m_Window, "Failed to create GLFW window!");
        LOG_INFO("Created GLFW window ... (Application: '{}', Size: {}x{})", m_Spec.Title, m_Spec.Width, m_Spec.Height);
    }

    void Window::CreateVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* surface)
    {
        VK_VERIFY((vk::Result)(glfwCreateWindowSurface(instance, m_Window, nullptr, (VkSurfaceKHR*)surface)));
        LOG_INFO("Created Vulkan surface for window ...");

        glfwSetFramebufferSizeCallback(m_Window, GLFW_FramebufferResizeCallback);
        LOG_INFO("Set framebuffer resize callback ...");

        // Update width and height with current framebuffer dimensions
        int width  = 0;
        int height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);
        SetWidth((u32)width);
        SetHeight((u32)height);
    }

    void Window::Shutdown()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        m_Window = nullptr;
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    void Window::WaitEvents()
    {
        glfwWaitEvents();
    }

    bool Window::ShouldClose()
    {
        if ((glfwWindowShouldClose(m_Window)) || (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS))
        {
            LOG_INFO("Closed GLFW window ...");
            return true;
        }

        return false;
    }

    [[nodiscard]] std::vector<const char*> Window::GetInstanceExtensions()
    {
        u32          count          = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        ASSERT(glfwExtensions && count > 0, "glfwGetRequiredInstanceExtensions failed ...");

        return { glfwExtensions, glfwExtensions + count };
    }

    // ----- Private -----

    void Window::GLFW_ErrorCallback(Engine::i32 errorCode, const char* description)
    {
        LOG_ERROR("GLFW::ErrorCallback: {} (Code: {})", description, errorCode);
        ASSERT(false, "Caught an error in GLFW::ErrorCallback!");
    }

    void Window::GLFW_FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        // Only one window is supported
        ASSERT(window == Engine::Platform::Window::GetHandle(),
               "GLFW::FramebufferResizeCallback: Received the wrong window ...");

        if (width == 0 || height == 0)
        {
            Engine::Platform::Window::SetMinimizeFlag(true);
            return;
        }

        if (Engine::Platform::Window::IsMinimized())
        {
            Engine::Platform::Window::SetMinimizeFlag(false);
        }

        if (Engine::Platform::Window::GetWidth() != (Engine::u32)width
            || Engine::Platform::Window::GetHeight() != (Engine::u32)height)
        {
            Engine::Platform::Window::SetWidth((Engine::u32)width);
            Engine::Platform::Window::SetHeight((Engine::u32)height);

            // Set resize flag. Swapchain needs to reset this
            Engine::Platform::Window::SetResizeFlag(true);
        }
    }

    void Window::SetMinimizeFlag(b8 flag)
    {
        m_IsMinimized = flag;
        LOG_VERBOSE("Window::MinizeFlag got set to '{}' ...", flag);
    }

    void Window::SetResizeFlag(b8 flag)
    {
        // Early return if nothing has changed
        // Swapchain was probably not recreated yet
        if (flag == m_GotResized)
        {
            return;
        }

        m_GotResized = flag;
        LOG_VERBOSE("Window::ResizeFlag got set to '{}' ...", flag);
    }
}
