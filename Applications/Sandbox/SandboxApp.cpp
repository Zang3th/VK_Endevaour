#include "SandboxApp.hpp"

#include <Core/Timer.hpp>

#include <Debug/Log.hpp>

#include <Graphics/Import/ObjLoader.hpp>

#include <Graphics/Vulkan/VulkanRenderer.hpp>

#include <Platform/Window.hpp>

Sandbox::Sandbox()
{
    Engine::Platform::Window::Init({ .Title = "Sandbox", .Width = 1920, .Height = 1080 });
}

Sandbox::~Sandbox()
{
    Engine::Platform::Window::Shutdown();
}

void Sandbox::Run()
{
    // Initialize timer
    Engine::Core::Timer timer;

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

    // Log startup time
    LOG_PERF("Engine startup time was {} ...", timer.GetEngineTotalRuntimeString());
    timer.SyncFrame();

    while (!Engine::Platform::Window::ShouldClose())
    {
        Engine::Platform::Window::PollEvents();

        if (Engine::Platform::Window::IsMinimized())
        {
            Engine::Platform::Window::WaitEvents();
            timer.SyncFrame();
            continue;
        }

        auto frame = vkRenderer.BeginFrame(pipelineID);

        // Check if frame can't be rendered
        if (!frame.IsValid())
        {
            timer.SyncFrame();
            continue;
        }

        // If frame is valid, tick timer and draw it
        timer.Tick();
        vkRenderer.DrawFrame(frame, timer.GetFrameTiming());
    }

    // Wait for device idle
    vkRenderer.WaitForDevice();

    // Log some stats
    LOG_PERF("Engine runtime was {} with an average of {} ...",
             timer.GetEngineTotalRuntimeString(),
             timer.GetEngineFPSAverageString());
}
