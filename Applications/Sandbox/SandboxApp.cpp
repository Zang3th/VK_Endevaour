#include "SandboxApp.hpp"

#include <Graphics/Import/ObjLoader.hpp>

#include <Graphics/Vulkan/VulkanRenderer.hpp>

#include <Platform/Window.hpp>

Sandbox::Sandbox()
{
    Engine::Platform::Window::Init({ .Title = "Sandbox", .Width = 1080, .Height = 1080 });
}

Sandbox::~Sandbox()
{
    Engine::Platform::Window::Shutdown();
}

void Sandbox::Run()
{
    // Initialize renderer
    Engine::Graphics::VulkanRenderer vkRenderer;

    // Load shader
    const Engine::u32 vertexID =
        vkRenderer.LoadShader(vk::ShaderStageFlagBits::eVertex, "Applications/Sandbox/Shaders/Vert.spv");
    const Engine::u32 fragmentID =
        vkRenderer.LoadShader(vk::ShaderStageFlagBits::eFragment, "Applications/Sandbox/Shaders/Frag.spv");

    // Create pipeline
    const Engine::u32 pipelineID = vkRenderer.CreatePipeline(vertexID, fragmentID);

    // Load mesh
    const Engine::Graphics::Mesh mesh =
        Engine::Graphics::ObjLoader::LoadMeshFromFile("Applications/Sandbox/Models/viking_room.obj");

    // Create 'hello_world_triangle' mesh
    // const Engine::Graphics::Mesh mesh;
    // mesh.Vertices = { { .Position = { +0.0f, -0.5f, 0.0f }, .Color = { 1, 0, 0 }, .TexCoord = { 0, 0 } },
    //                   { .Position = { +0.5f, +0.5f, 0.0f }, .Color = { 0, 1, 0 }, .TexCoord = { 1, 0 } },
    //                   { .Position = { -0.5f, +0.5f, 0.0f }, .Color = { 0, 0, 1 }, .TexCoord = { 0, 1 } } };
    // mesh.Indices  = { 0, 1, 2 };

    // Create model from mesh
    const Engine::u32 modelID = vkRenderer.CreateModel(&mesh);

    // Assign model to pipeline
    vkRenderer.AssignModelToPipeline(modelID, pipelineID);

    while (!Engine::Platform::Window::ShouldClose())
    {
        Engine::Platform::Window::PollEvents();

        if (Engine::Platform::Window::IsMinimized())
        {
            Engine::Platform::Window::WaitEvents();
            continue;
        }

        // Draw
        vkRenderer.DrawFrame(pipelineID);
    }

    // Wait for device idle
    vkRenderer.WaitForDevice();
}
