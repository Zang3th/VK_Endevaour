#include "SandboxApp.hpp"

#include <Platform/Window.hpp>

#include <Graphics/Vulkan/VulkanRenderer.hpp>
#include <Graphics/Import/ObjLoader.hpp>

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

    // Load mesh
    // Engine::Mesh mesh = Engine::ObjLoader::LoadMeshFromFile("Models/viking_room.obj");

    // Create 'hello_world_triangle' mesh
    Engine::Mesh mesh;
    mesh.Vertices =
    {
        { .Position = { 0.0f, -0.5f, 0.0f }, .Color = {1,0,0}, .TexCoord = {0,0} },
        { .Position = { 0.5f,  0.5f, 0.0f }, .Color = {0,1,0}, .TexCoord = {1,0} },
        { .Position = {-0.5f,  0.5f, 0.0f }, .Color = {0,0,1}, .TexCoord = {0,1} }
    };
    mesh.Indices  = { 0, 1, 2 };

    // Create model from mesh
    u32 modelID = vkRenderer.CreateModel(&mesh);

    // Assign model to pipeline
    vkRenderer.AssignPipeline(modelID, pipelineID);

    while(!Engine::Window::ShouldClose())
    {
        Engine::Window::PollEvents();

        // Draw
        vkRenderer.DrawFrame(pipelineID);
    }

    // Wait for device idle
    vkRenderer.WaitForDevice();
}
