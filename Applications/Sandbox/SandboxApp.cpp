#include "SandboxApp.hpp"

#include <Core/Window.hpp>

#include <Graphics/Vulkan/VulkanRenderer.hpp>

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

    // TODO: Create pipeline
    // vkRenderer.CreatePipeline("ForwardRenderingPipeline", "DefaultVert", "DefaultFrag");

    // TODO: Load model
    // vkRenderer.LoadObjModel("VikingRoom", "Models/viking_room.obj");
    // vkRenderer.AssignPipeline("VikingRoom", "ForwardRenderingPipeline");

    while(!Engine::Window::ShouldClose())
    {
        Engine::Window::PollEvents();

        // TODO: Draw frame
        // vkRenderer.DrawFrame();
    }
}
