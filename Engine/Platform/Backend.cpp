#include "Backend.hpp"

#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"

#include "Platform/Window.hpp"

// Vulkan needs to be included by now
#include "Vendor/glfw/include/GLFW/glfw3.h"

namespace Engine::Platform
{
    namespace
    {
        constexpr i32 KeyToGLFW(Key key)
        {
            switch (key)
            {
            case Key::W:
                return GLFW_KEY_W;
            case Key::A:
                return GLFW_KEY_A;
            case Key::S:
                return GLFW_KEY_S;
            case Key::D:
                return GLFW_KEY_D;
            case Key::Space:
                return GLFW_KEY_SPACE;
            case Key::LeftShift:
                return GLFW_KEY_LEFT_SHIFT;
            case Key::LeftControl:
                return GLFW_KEY_LEFT_CONTROL;
            case Key::Escape:
                return GLFW_KEY_ESCAPE;
            case Key::Count:
                break;
            }
            ASSERT(false, "KeyToGLFW: Unknown input!");
            return -1;
        }

        constexpr i32 MouseToGLFW(MouseButton mouseButton)
        {
            switch (mouseButton)
            {
            case MouseButton::Left:
                return GLFW_MOUSE_BUTTON_LEFT;
            case MouseButton::Right:
                return GLFW_MOUSE_BUTTON_RIGHT;
            case MouseButton::Middle:
                return GLFW_MOUSE_BUTTON_MIDDLE;
            case MouseButton::Count:
                break;
            }
            ASSERT(false, "MouseToGLFW: Unknown input!");
            return -1;
        }
    }

    // ----- Backend -----

    Backend::Backend(const WindowInitSpecification& initSpec)
    {
        // Initialize GLFW
        ASSERT(glfwSetErrorCallback(&GLFW_ErrorCallback) == nullptr, "Failed to set GLFW_ErrorCallback!");
        ASSERT(glfwInit(), "Failed to initialize GLFW!");
        LOG_INFO("Initialized GLFW ...");

        // Create window and set user pointer
        m_Window     = Scope<Window>(new Window(initSpec));
        m_GLFWHandle = m_Window->m_GLFWHandle;
        glfwSetWindowUserPointer(m_GLFWHandle, this);

        // Set callbacks
        ASSERT(glfwSetFramebufferSizeCallback(m_GLFWHandle, &GLFW_FramebufferResizeCallback) == nullptr,
               "Failed to set GLFW_FramebufferResizeCallback!");
        ASSERT(glfwSetScrollCallback(m_GLFWHandle, &GLFW_ScrollCallback) == nullptr,
               "Failed to set GLFW_ScrollCallback!");
        LOG_INFO("Initialized GLFW callbacks ...");

        // Update width and height with current framebuffer dimensions
        i32 width  = 0;
        i32 height = 0;
        glfwGetFramebufferSize(m_GLFWHandle, &width, &height);
        m_Window->m_FramebufferExtent = { .Width = (u32)width, .Height = (u32)height };
    }

    Backend::~Backend()
    {
        LOG_INFO("Backend::Destructor() ...");
        m_Window.reset();
        glfwTerminate();
    }

    void Backend::WaitEvents()
    {
        glfwWaitEvents();
        m_CursorTracker.IsValid = false;
    }

    [[nodiscard]] FrameInput Backend::PollEvents()
    {
        glfwPollEvents();

        // Get keys down
        std::bitset<KEY_COUNT> keysDown;
        for (u8 keyIndex = 0; keyIndex < KEY_COUNT; keyIndex++)
        {
            keysDown.set(keyIndex, glfwGetKey(m_GLFWHandle, KeyToGLFW((Key)keyIndex)) == GLFW_PRESS);
        }

        // Get mouse buttons down
        std::bitset<MOUSE_BUTTON_COUNT> mouseButtonsDown;
        for (u8 mouseIndex = 0; mouseIndex < MOUSE_BUTTON_COUNT; mouseIndex++)
        {
            mouseButtonsDown.set(mouseIndex,
                                 glfwGetMouseButton(m_GLFWHandle, MouseToGLFW((MouseButton)mouseIndex)) == GLFW_PRESS);
        }

        // Get current cursor position
        f64 xPos = 0;
        f64 yPos = 0;
        glfwGetCursorPos(m_GLFWHandle, &xPos, &yPos);
        const glm::vec2 cursorPosition(xPos, yPos);

        // Calculate cursor delta
        glm::vec2 cursorDelta = cursorPosition - m_CursorTracker.PreviousPosition;

        // Skip this frame for cursor tracking (e.g. after minimizing)
        if (!m_CursorTracker.IsValid)
        {
            cursorDelta             = { 0.0f, 0.0f };
            m_CursorTracker.IsValid = true;
        }
        m_CursorTracker.PreviousPosition = cursorPosition;

        // Get scroll delta
        const glm::vec2 scrollDelta = m_ScrollAccumulator.Delta;
        m_ScrollAccumulator.Delta   = { 0.0f, 0.0f };

        return {
            .KeysDown         = keysDown,
            .MouseButtonsDown = mouseButtonsDown,
            .CursorPosition   = cursorPosition,
            .CursorDelta      = cursorDelta,
            .ScrollDelta      = scrollDelta,
        };
    }

    void Backend::GLFW_ErrorCallback(i32 errorCode, const char* description)
    {
        LOG_ERROR("GLFW_ErrorCallback: {} (Code: {})", description, errorCode);
        ASSERT(false, "Caught an error in GLFW_ErrorCallback!");
    }

    void Backend::GLFW_FramebufferResizeCallback(GLFWwindow* window, i32 width, i32 height)
    {
        void* backendPtr = glfwGetWindowUserPointer(window);
        ASSERT(backendPtr, "User pointer was invalid!");
        ((Backend*)backendPtr)->m_Window->m_FramebufferExtent = { .Width = (u32)width, .Height = (u32)height };
    }

    void Backend::GLFW_ScrollCallback(GLFWwindow* window, f64 x, f64 y)
    {
        void* backendPtr = glfwGetWindowUserPointer(window);
        ASSERT(backendPtr, "User pointer was invalid!");
        ((Backend*)backendPtr)->m_ScrollAccumulator.Delta += glm::vec2((f32)x, (f32)y);
    }

    // ----- Window -----

    Window::Window(const WindowInitSpecification& initSpec) : m_InitSpec(initSpec)
    {
        ASSERT(m_InitSpec.Width > 0 && m_InitSpec.Height > 0, "Window width and height must be greater than zero!");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_GLFWHandle =
            glfwCreateWindow((int)m_InitSpec.Width, (int)m_InitSpec.Height, m_InitSpec.Title.c_str(), nullptr, nullptr);
        ASSERT(m_GLFWHandle, "Failed to create GLFW window!");
        LOG_INFO("Created GLFW window ... (Application: '{}', Size: {}x{})",
                 m_InitSpec.Title,
                 m_InitSpec.Width,
                 m_InitSpec.Height);
    }

    Window::~Window()
    {
        LOG_INFO("Window::Destructor() ...");
        glfwDestroyWindow(m_GLFWHandle);
    }

    void Window::CreateVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* surface) const
    {
        VK_VERIFY((vk::Result)(glfwCreateWindowSurface(instance, m_GLFWHandle, nullptr, (VkSurfaceKHR*)surface)));
        LOG_INFO("Created Vulkan surface for GLFW window ...");
    }

    [[nodiscard]] std::vector<const char*> Window::GetRequiredInstanceExtensions() const
    {
        u32 count = 0;

        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        ASSERT(glfwExtensions && count > 0, "Failed to get required instance extensions via GLFW!");

        return { glfwExtensions, glfwExtensions + count };
    }

    [[nodiscard]] b8 Window::ShouldClose() const
    {
        return glfwWindowShouldClose(m_GLFWHandle) == GLFW_TRUE;
    }
}
