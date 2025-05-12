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
    u32 vertexID   = vkRenderer.LoadShader(vk::ShaderStageFlagBits::eVertex,   "Shaders/Vert.spv");
    u32 fragmentID = vkRenderer.LoadShader(vk::ShaderStageFlagBits::eFragment, "Shaders/Frag.spv");

    // Create pipeline
    u32 pipelineID = vkRenderer.CreatePipeline(vertexID, fragmentID);

    // Load model
    u32 modelID = vkRenderer.LoadModel("Models/viking_room.obj");
    vkRenderer.AssignPipeline(modelID, pipelineID);

    while(!Engine::Window::ShouldClose())
    {
        Engine::Window::PollEvents();

        // Draw frame
        vkRenderer.DrawFrame(pipelineID);
    }
}
