#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp> // This needs to be included before GLFW
#include <GLFW/glfw3.h>

namespace Engine
{
    struct WindowSpecification
    {
        std::string Title  = "DefaultWindowTitle";
        u32         Width  = 1080;
        u32         Height = 1080;
    };

    class Window
    {
        public:
            static void Init(const WindowSpecification& spec);
            static void Shutdown();
            ~Window() = delete;

            static void PollEvents();
            static bool ShouldClose();
            [[nodiscard]] static GLFWwindow*        GetHandle() { return m_Window;      }
            [[nodiscard]] static const std::string& GetTitle()  { return m_Spec.Title;  }
            [[nodiscard]] static u32                GetWidth()  { return m_Spec.Width;  }
            [[nodiscard]] static u32                GetHeight() { return m_Spec.Height; }

        private:
            inline static GLFWwindow*         m_Window = nullptr;
            inline static WindowSpecification m_Spec   = WindowSpecification();
    };
}
