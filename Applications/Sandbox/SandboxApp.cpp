#include "SandboxApp.hpp"

#include <Core/Window.hpp>
#include <Graphics/Vulkan/VulkanContext.hpp>

void Sandbox::Run()
{
    Engine::Window::Init({ .Title = "Sandbox" });
    Engine::VulkanContext vkContext;

    while(!Engine::Window::ShouldClose())
    {
        Engine::Window::PollEvents();
    }

    Engine::Window::Shutdown();
}
