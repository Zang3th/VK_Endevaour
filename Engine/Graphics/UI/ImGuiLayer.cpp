#include "ImGuiLayer.hpp"

#include "Debug/Log.hpp"

#include "Platform/Window.hpp"

#include "Vendor/imgui/imgui.h"
#include "Vendor/imgui/imgui_impl_glfw.h"
#include "Vendor/imgui/imgui_impl_vulkan.h"

namespace
{
    void ImGuiVkResultCallback(VkResult result)
    {
        if (result == VK_SUCCESS)
        {
            return;
        }

        LOG_ERROR("ImGui::VkResultCallback: {}", vk::to_string((vk::Result)result));
        ASSERT(false, "Caught an error in ImGui::VkResultCallback!");
    }
}

namespace Engine::Graphics
{
    // ----- Public -----

    ImGuiLayer::ImGuiLayer(VulkanContext* context) : m_Context(context)
    {
        Init();
    }

    ImGuiLayer::~ImGuiLayer()
    {
        LOG_INFO("ImGuiLayer::Destructor() ...");
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
    }

    void ImGuiLayer::RenderFrame(vk::CommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    // ----- Private -----

    void ImGuiLayer::CreateDescriptorPool()
    {
        constexpr u32 descriptorCount = 1000;

        const DescriptorPoolSpecification spec{
            .Flags     = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .MaxSets   = descriptorCount,
            .PoolSizes = { { .type = vk::DescriptorType::eSampler, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eSampledImage, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageImage, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eUniformTexelBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageTexelBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eUniformBufferDynamic, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eStorageBufferDynamic, .descriptorCount = descriptorCount },
                           { .type = vk::DescriptorType::eInputAttachment, .descriptorCount = descriptorCount } },
        };

        m_DescriptorPool = MakeScope<VulkanDescriptorPool>(m_Context->GetDevice()->GetHandle(), spec);
    }

    void ImGuiLayer::Init()
    {
        CreateDescriptorPool();

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // Dynamic rendering specification
        const vk::Format                      format = m_Context->GetSwapchain()->GetProperties().SurfaceFormat.format;
        const vk::PipelineRenderingCreateInfo renderingInfo{
            .colorAttachmentCount    = m_Context->GetSwapchain()->GetProperties().GetColorAttachmentCount(),
            .pColorAttachmentFormats = &format
        };

        const ImGui_ImplVulkan_PipelineInfo pipelineInfo{
            .RenderPass                  = nullptr,
            .Subpass                     = 0,
            .ExtraDynamicStates          = {},
            .PipelineRenderingCreateInfo = renderingInfo,
        };

        constexpr vk::DeviceSize IMGUI_MIN_ALLOCATION_SIZE = 1048576; // ImGui best practice

        // Setup ImGui renderer backend
        ImGui_ImplVulkan_InitInfo initInfo{ .ApiVersion     = m_Context->GetApiVersion(),
                                            .Instance       = m_Context->GetInstance(),
                                            .PhysicalDevice = m_Context->GetDevice()->GetPhysicalDevice()->GetHandle(),
                                            .Device         = m_Context->GetDevice()->GetHandle(),
                                            .QueueFamily    = m_Context->GetDevice()->GetGraphicsQueueFamily(),
                                            .Queue          = m_Context->GetDevice()->GetGraphicsQueue(),
                                            .DescriptorPool = m_DescriptorPool->GetHandle(),
                                            .DescriptorPoolSize =
                                                0, // ImGui backend would create an own pool with values > 0
                                            .MinImageCount = m_Context->GetSwapchain()->GetProperties().MinImageCount,
                                            .ImageCount    = m_Context->GetSwapchain()->GetImageCount(),
                                            .PipelineCache = nullptr,
                                            .PipelineInfoMain           = pipelineInfo,
                                            .UseDynamicRendering        = true,
                                            .Allocator                  = nullptr,
                                            .CheckVkResultFn            = &ImGuiVkResultCallback,
                                            .MinAllocationSize          = IMGUI_MIN_ALLOCATION_SIZE,
                                            .CustomShaderVertCreateInfo = {},
                                            .CustomShaderFragCreateInfo = {} };

        ASSERT(ImGui_ImplGlfw_InitForVulkan(Platform::Window::GetHandle(), true),
               "ImGui_ImplGlfw_InitForVulkan failed!");
        ASSERT(ImGui_ImplVulkan_Init(&initInfo), "ImGui_ImplVulkan_Init failed!");

        LOG_INFO("Initialized ImGui ...");
    }
}
