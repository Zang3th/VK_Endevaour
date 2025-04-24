#include "Core/Window.hpp"
#include "Debug/Log.hpp"

namespace Engine
{
    // ----- Public -----

    void Window::Init(const WindowSpecification& spec)
    {
        ASSERT(glfwInit(), "Failed to initialize GLFW: {}", glfwGetError(nullptr));
        LOG_INFO("Initialized GLFW ...");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow((int)spec.Width, (int)spec.Height, spec.Title.c_str(), nullptr, nullptr);
        ASSERT(m_Window, "Failed to create GLFW window: {}", glfwGetError(nullptr));
        LOG_INFO("Created GLFW window for application '{}' ...", spec.Title);
    }

    void Window::Shutdown()
    {
        glfwTerminate();
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    bool Window::ShouldClose()
    {
        if((glfwWindowShouldClose(m_Window)) || (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS))
        {
            LOG_INFO("Closed GLFW window ...");
            return true;
        }

        return false;
    }
}
