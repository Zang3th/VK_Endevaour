#pragma once

#include "Core/Types.hpp"

#include <string>
#include <vector>

struct GLFWwindow;

namespace vk
{
    class Instance;
    class SurfaceKHR;
}

namespace Engine::Graphics
{
    class VulkanSwapchain;
}

namespace Engine::Platform
{
    struct WindowSpecification
    {
        std::string Title  = "DefaultWindowTitle";
        u32         Width  = 0;
        u32         Height = 0;
    };

    class Window
    {
    public:
        Window() = delete;
        static void Init(const WindowSpecification& spec);
        static void CreateVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* surface);
        static void Shutdown();

        static void PollEvents();
        static void WaitEvents();
        static bool ShouldClose();

        [[nodiscard]] static std::vector<const char*> GetInstanceExtensions();

        [[nodiscard]] static GLFWwindow*        GetHandle() { return m_Window; }
        [[nodiscard]] static const std::string& GetTitle() { return m_Spec.Title; }
        [[nodiscard]] static u32                GetWidth() { return m_Spec.Width; }
        [[nodiscard]] static u32                GetHeight() { return m_Spec.Height; }
        [[nodiscard]] static b8                 IsMinimized() { return m_IsMinimized; }
        [[nodiscard]] static b8                 GotResized() { return m_GotResized; }

    private:
        friend class Engine::Graphics::VulkanSwapchain;

        static void GLFW_ErrorCallback(Engine::i32 errorCode, const char* description);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* window, int width, int height);

        static void SetMinimizeFlag(b8 flag);
        static void SetResizeFlag(b8 flag);
        static void SetWidth(u32 width) { m_Spec.Width = width; };
        static void SetHeight(u32 height) { m_Spec.Height = height; };

        inline static GLFWwindow*         m_Window      = nullptr;
        inline static WindowSpecification m_Spec        = WindowSpecification();
        inline static b8                  m_IsMinimized = false;
        inline static b8                  m_GotResized  = false;
    };
}
