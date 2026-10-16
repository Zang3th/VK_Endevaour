#pragma once

#include "Core/Types.hpp"

#include "Vendor/imgui/imgui.h"

#include <array>

namespace Engine::Graphics
{
    template <typename T>
    struct MenuOption
    {
        const char* Label;
        T           Value;
    };

    class ImGuiExtensions
    {
    public:
        ImGuiExtensions() = delete;

        template <typename T, usize N>
        static void RadioMenu(const char* label, T& current, const std::array<MenuOption<T>, N>& options)
        {
            if (!ImGui::BeginMenu(label))
            {
                return;
            }

            for (const MenuOption<T>& option : options)
            {
                if (ImGui::MenuItem(option.Label, nullptr, current == option.Value))
                {
                    current = option.Value;
                }
            }

            ImGui::EndMenu();
        }
    };
}
