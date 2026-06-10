#include "Debug/Log.hpp"

#include "Window.hpp"

namespace
{
    void GLFW_ErrorCallback(Engine::i32 errorCode, const char* description)
    {
        LOG_ERROR("GLFW::ErrorCallback: {} (Code: {})", description, errorCode);
        ASSERT(false, "Caught an error in GLFW::ErrorCallback!");
    }
}

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

    void Window::UpdateFramebufferSize()
    {
        glfwGetFramebufferSize(m_Window, (int*)&m_Spec.Width, (int*)&m_Spec.Height);
    }
}
