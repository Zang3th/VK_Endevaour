#pragma once

#include "Core/Memory.hpp"

#include "Platform/Input.hpp"
#include "Platform/Window.hpp"

namespace Engine::Platform
{
    class Backend
    {
    public:
        // Initializes GLFW, creates the window and registers all callbacks
        explicit Backend(const WindowSpecification& spec);

        // Destroys the window before terminating GLFW
        ~Backend();

        Backend(const Backend&)            = delete;
        Backend& operator=(const Backend&) = delete;
        Backend(Backend&&)                 = delete;
        Backend& operator=(Backend&&)      = delete;

        // Blocks until an event arrives
        void WaitEvents();

        // Polls events: Call exactly once per frame
        [[nodiscard]] FrameInput PollEvents();

        [[nodiscard]] const Window& GetWindow() const { return *m_Window; }

    private:
        static void GLFW_ErrorCallback(i32 errorCode, const char* description);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* window, i32 width, i32 height);
        static void GLFW_ScrollCallback(GLFWwindow* window, f64 x, f64 y);

        struct CursorTracker
        {
            glm::vec2 PreviousPosition{};
            b8        IsValid = false;
        };

        struct ScrollAccumulator
        {
            glm::vec2 Delta{};
        };

        Scope<Window>     m_Window;
        CursorTracker     m_CursorTracker;
        ScrollAccumulator m_ScrollAccumulator;
    };
}
