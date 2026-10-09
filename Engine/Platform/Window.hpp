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

namespace Engine::Platform
{
    struct WindowSpecification
    {
        std::string Title = "DefaultWindowTitle";

        // Screen coordinates, may differ from framebuffer pixels (DPI)
        u32 Width  = 0;
        u32 Height = 0;
    };

    struct FramebufferExtent
    {
        u32 Width  = 0;
        u32 Height = 0;

        b8                         operator==(const FramebufferExtent&) const = default;
        [[nodiscard]] constexpr b8 IsEmpty() const { return Width == 0 || Height == 0; }
    };

    class Window
    {
    public:
        ~Window();

        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&)                 = delete;
        Window& operator=(Window&&)      = delete;

        void CreateVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR* surface) const;

        [[nodiscard]] std::vector<const char*>   GetRequiredInstanceExtensions() const;
        [[nodiscard]] b8                         ShouldClose() const;
        [[nodiscard]] FramebufferExtent          GetFramebufferExtent() const { return m_FramebufferExtent; }
        [[nodiscard]] GLFWwindow*                GetGLFWHandle() const { return m_GLFWHandle; }
        [[nodiscard]] const WindowSpecification& GetSpecification() const { return m_Spec; }

    private:
        friend class Backend;
        explicit Window(const WindowSpecification& spec);

        GLFWwindow*         m_GLFWHandle = nullptr;
        WindowSpecification m_Spec;
        FramebufferExtent   m_FramebufferExtent;
    };
}
