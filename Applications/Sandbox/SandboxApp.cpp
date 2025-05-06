#include "SandboxApp.hpp"

#include <Core/Window.hpp>
#include <Graphics/Vulkan/VulkanRenderer.hpp>
// #include <Graphics/Vulkan/VulkanModel.hpp>
// #include <Graphics/Vulkan/VulkanShader.hpp>
// #include <Graphics/Vulkan/VulkanPipeline.hpp>

Sandbox::Sandbox()
{
    Engine::Window::Init({ .Title = "Sandbox" });
}

Sandbox::~Sandbox()
{
    Engine::Window::Shutdown();
}

void Sandbox::Run()
{
    // Initialize renderer
    Engine::VulkanRenderer vkRenderer;

    // Load shader
    vkRenderer.LoadShader("DefaultVert", vk::ShaderStageFlagBits::eVertex, "Shaders/Vert.spv");
    vkRenderer.LoadShader("DefaultFrag", vk::ShaderStageFlagBits::eFragment, "Shaders/Frag.spv");

    // Load model

    // TODO: Create pipeline

    // TODO: Submit model and pipeline to renderer

    while(!Engine::Window::ShouldClose())
    {
        Engine::Window::PollEvents();

        // TODO: Let renderer draw frame
    }
}
