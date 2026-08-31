#include "ImGuiLayer.hpp"

#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"
#include "Graphics/Vulkan/VulkanDescriptorPool.hpp"
#include "Graphics/Vulkan/VulkanDevice.hpp"
#include "Graphics/Vulkan/VulkanGlobals.hpp"
#include "Graphics/Vulkan/VulkanSwapchain.hpp"

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
        ConfigureStyle();
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

        const auto* swapchain = m_Context->GetSwapchain();
        const auto* device    = m_Context->GetDevice();

        // Dynamic rendering specification
        const vk::Format                      format = swapchain->GetProperties().SurfaceFormat.format;
        const vk::PipelineRenderingCreateInfo renderingInfo{ .colorAttachmentCount    = GLOBAL_COLOR_ATTACHMENT_COUNT,
                                                             .pColorAttachmentFormats = &format };

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
                                            .PhysicalDevice = device->GetPhysicalDevice()->GetHandle(),
                                            .Device         = device->GetHandle(),
                                            .QueueFamily    = device->GetGraphicsQueueFamily(),
                                            .Queue          = device->GetGraphicsQueue(),
                                            .DescriptorPool = m_DescriptorPool->GetHandle(),
                                            .DescriptorPoolSize =
                                                0, // ImGui backend would create an own pool with values > 0
                                            .MinImageCount              = swapchain->GetProperties().MinImageCount,
                                            .ImageCount                 = swapchain->GetImageCount(),
                                            .PipelineCache              = nullptr,
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

    void ImGuiLayer::ConfigureStyle()
    {
        ImGui::StyleColorsDark();

        constexpr ImVec4 background     = { 0.10f, 0.105f, 0.11f, 1.00f };
        constexpr ImVec4 backgroundDark = { 0.06f, 0.065f, 0.07f, 1.00f };
        constexpr ImVec4 surface        = { 0.20f, 0.205f, 0.21f, 1.00f };
        constexpr ImVec4 surfaceHovered = { 0.30f, 0.305f, 0.31f, 1.00f };
        constexpr ImVec4 surfaceActive  = { 0.15f, 0.155f, 0.16f, 1.00f };
        constexpr ImVec4 border         = { 0.28f, 0.285f, 0.29f, 1.00f };
        constexpr ImVec4 text           = { 0.95f, 0.95f, 0.95f, 1.00f };
        constexpr ImVec4 textDisabled   = { 0.55f, 0.555f, 0.56f, 1.00f };
        constexpr ImVec4 accent         = { 0.20f, 0.25f, 0.55f, 1.00f };
        constexpr ImVec4 accentHovered  = { 0.35f, 0.40f, 0.70f, 1.00f };
        constexpr ImVec4 accentActive   = { 0.45f, 0.50f, 0.80f, 1.00f };
        constexpr ImVec4 transparent    = { 0.00f, 0.00f, 0.00f, 0.00f };

        ImGuiStyle& style  = ImGui::GetStyle();
        auto&       colors = style.Colors;

        // Text
        colors[ImGuiCol_Text]           = text;
        colors[ImGuiCol_TextDisabled]   = textDisabled;
        colors[ImGuiCol_TextLink]       = accentActive;
        colors[ImGuiCol_TextSelectedBg] = ImVec4{ accent.x, accent.y, accent.z, 0.55f };

        // Window backgrounds and borders
        colors[ImGuiCol_WindowBg]     = background;
        colors[ImGuiCol_ChildBg]      = transparent;
        colors[ImGuiCol_PopupBg]      = backgroundDark;
        colors[ImGuiCol_Border]       = border;
        colors[ImGuiCol_BorderShadow] = transparent;
        colors[ImGuiCol_MenuBarBg]    = backgroundDark;

        // Window titles
        colors[ImGuiCol_TitleBg]          = surfaceActive;
        colors[ImGuiCol_TitleBgActive]    = surfaceActive;
        colors[ImGuiCol_TitleBgCollapsed] = backgroundDark;

        // Frames and input fields
        colors[ImGuiCol_FrameBg]         = surface;
        colors[ImGuiCol_FrameBgHovered]  = surfaceHovered;
        colors[ImGuiCol_FrameBgActive]   = surfaceActive;
        colors[ImGuiCol_InputTextCursor] = text;

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg]          = backgroundDark;
        colors[ImGuiCol_ScrollbarGrab]        = surface;
        colors[ImGuiCol_ScrollbarGrabHovered] = surfaceHovered;
        colors[ImGuiCol_ScrollbarGrabActive]  = accent;

        // Selection controls and sliders
        colors[ImGuiCol_CheckMark]          = text;
        colors[ImGuiCol_CheckboxSelectedBg] = accent;
        colors[ImGuiCol_SliderGrab]         = accent;
        colors[ImGuiCol_SliderGrabActive]   = accentActive;

        // Buttons
        colors[ImGuiCol_Button]        = surface;
        colors[ImGuiCol_ButtonHovered] = surfaceHovered;
        colors[ImGuiCol_ButtonActive]  = surfaceActive;

        // Headers, tree nodes and selectable items
        colors[ImGuiCol_Header]        = surface;
        colors[ImGuiCol_HeaderHovered] = surfaceHovered;
        colors[ImGuiCol_HeaderActive]  = surfaceActive;
        colors[ImGuiCol_TreeLines]     = border;

        // Separators and resize grips
        colors[ImGuiCol_Separator]         = border;
        colors[ImGuiCol_SeparatorHovered]  = accentHovered;
        colors[ImGuiCol_SeparatorActive]   = accentActive;
        colors[ImGuiCol_ResizeGrip]        = accent;
        colors[ImGuiCol_ResizeGripHovered] = accentHovered;
        colors[ImGuiCol_ResizeGripActive]  = accentActive;

        // Tabs
        colors[ImGuiCol_Tab]                       = surfaceActive;
        colors[ImGuiCol_TabHovered]                = surfaceHovered;
        colors[ImGuiCol_TabSelected]               = surface;
        colors[ImGuiCol_TabSelectedOverline]       = accentActive;
        colors[ImGuiCol_TabDimmed]                 = backgroundDark;
        colors[ImGuiCol_TabDimmedSelected]         = surfaceActive;
        colors[ImGuiCol_TabDimmedSelectedOverline] = accent;

        // Plots
        colors[ImGuiCol_PlotLines]            = accentActive;
        colors[ImGuiCol_PlotLinesHovered]     = text;
        colors[ImGuiCol_PlotHistogram]        = accent;
        colors[ImGuiCol_PlotHistogramHovered] = accentActive;

        // Tables
        colors[ImGuiCol_TableHeaderBg]     = surface;
        colors[ImGuiCol_TableBorderStrong] = border;
        colors[ImGuiCol_TableBorderLight]  = surface;
        colors[ImGuiCol_TableRowBg]        = transparent;
        colors[ImGuiCol_TableRowBgAlt]     = ImVec4{ 1.00f, 1.00f, 1.00f, 0.03f };

        // Drag and drop
        colors[ImGuiCol_DragDropTarget]   = accentActive;
        colors[ImGuiCol_DragDropTargetBg] = ImVec4{ accent.x, accent.y, accent.z, 0.20f };

        // Navigation and overlays
        colors[ImGuiCol_UnsavedMarker]         = accentActive;
        colors[ImGuiCol_NavCursor]             = accentActive;
        colors[ImGuiCol_NavWindowingHighlight] = text;
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4{ 0.00f, 0.00f, 0.00f, 0.35f };
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4{ 0.00f, 0.00f, 0.00f, 0.55f };

        // Spacing and rounding
        style.WindowPadding     = { 12.0f, 12.0f };
        style.FramePadding      = { 8.0f, 5.0f };
        style.ItemSpacing       = { 8.0f, 6.0f };
        style.WindowRounding    = 6.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;
    }
}
