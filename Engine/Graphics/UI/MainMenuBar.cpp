#include "MainMenuBar.hpp"

#include "Graphics/UI/ImGuiExtensions.hpp"

#include "Graphics/Vulkan/VulkanRendererStructs.hpp"

#include "Vendor/imgui/imgui.h"

namespace
{
    constexpr std::array POLYGON_MODES{
        Engine::Graphics::MenuOption<vk::PolygonMode>{ .Label = "Fill", .Value = vk::PolygonMode::eFill },
        Engine::Graphics::MenuOption<vk::PolygonMode>{ .Label = "Wireframe", .Value = vk::PolygonMode::eLine },
        Engine::Graphics::MenuOption<vk::PolygonMode>{ .Label = "Points", .Value = vk::PolygonMode::ePoint }
    };

    constexpr std::array CULL_MODES{
        Engine::Graphics::MenuOption<vk::CullModeFlagBits>{ .Label = "Back", .Value = vk::CullModeFlagBits::eBack },
        Engine::Graphics::MenuOption<vk::CullModeFlagBits>{ .Label = "Front", .Value = vk::CullModeFlagBits::eFront },
        Engine::Graphics::MenuOption<vk::CullModeFlagBits>{ .Label = "None", .Value = vk::CullModeFlagBits::eNone }
    };
}

namespace Engine::Graphics
{
    // ----- Public -----

    void MainMenuBar::Render(Graphics::RenderSettings& settings)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        const bool isOpen = ImGui::BeginMainMenuBar();
        ImGui::PopStyleVar();

        if (!isOpen)
        {
            return;
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGuiExtensions::RadioMenu("Polygon Mode", settings.PolygonMode, POLYGON_MODES);
            ImGuiExtensions::RadioMenu("Cull Mode", settings.CullMode, CULL_MODES);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
