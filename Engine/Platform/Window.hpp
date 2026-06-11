#pragma once

#include "Core/Types.hpp"

#include "Vendor/glfw/include/GLFW/glfw3.h"

#include <vulkan/vulkan.hpp> // This needs to be included before GLFW

namespace Engine::Platform
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
        Window() = delete;
        static void Init(const WindowSpecification& spec);
        static void Shutdown();

        static void PollEvents();
        static void WaitEvents();
        static bool ShouldClose();
        static void UpdateFramebufferSize();

        [[nodiscard]] static GLFWwindow*        GetHandle() { return m_Window; }
        [[nodiscard]] static const std::string& GetTitle() { return m_Spec.Title; }
        [[nodiscard]] static u32                GetWidth() { return m_Spec.Width; }
        [[nodiscard]] static u32                GetHeight() { return m_Spec.Height; }
        [[nodiscard]] static b8                 IsMinimized() { return m_IsMinimized; }

        static void SetWidth(u32 width) { m_Spec.Width = width; };
        static void SetHeight(u32 height) { m_Spec.Height = height; };
        static void SetMinimize(b8 flag) { m_IsMinimized = flag; };

    private:
        inline static GLFWwindow*         m_Window      = nullptr;
        inline static WindowSpecification m_Spec        = WindowSpecification();
        inline static b8                  m_IsMinimized = false;
    };
}
