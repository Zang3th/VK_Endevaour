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
    const Engine::Graphics::Mesh cowMesh = Engine::Graphics::ObjLoader::LoadMeshFromFile(
        "Applications/Sandbox/Models/cow.obj", Engine::Graphics::ImportMode::eTopologyDebug);

    // Create 'hello_world_triangle' mesh
    const Engine::Graphics::Mesh triangleMesh{
        .Vertices = { { .Position = { +10.0f, +10.0f, -10.0f }, .Color = { 1, 0, 0 }, .TexCoord = { 0, 0 } },
                      { .Position = { -10.0f, +10.0f, -10.0f }, .Color = { 0, 1, 0 }, .TexCoord = { 0, 0 } },
                      { .Position = { +10.0f, -10.0f, -10.0f }, .Color = { 0, 0, 1 }, .TexCoord = { 0, 0 } },
                      { .Position = { -10.0f, -10.0f, -10.0f }, .Color = { 1, 1, 1 }, .TexCoord = { 0, 0 } } },
        // Project winding convention:
        // Mesh indices are authored clockwise when viewed from the geometric front side in model space.
        // The Vulkan projection flips clip-space Y (Projection[1][1] *= -1), which inverts the final
        // screen-space winding. Therefore the rasterizer is configured with FrontFace = eCounterClockwise.
        .Indices = { 0, 1, 2, 1, 3, 2 }
    };

    // Create models from meshes
    const Engine::u32 cowModel      = vkRenderer.CreateModel(&cowMesh);
    const Engine::u32 triangleModel = vkRenderer.CreateModel(&triangleMesh);

    // Assign models to pipeline
    vkRenderer.AssignModelToPipeline(cowModel, pipelineID);
    vkRenderer.AssignModelToPipeline(triangleModel, pipelineID);

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
}
