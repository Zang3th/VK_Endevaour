#pragma once

#include "Core/Types.hpp"

#include "Math/Math.hpp"

#include <bitset>
#include <cstddef>

namespace Engine::Platform
{
    enum class Key : u8
    {
        W,
        A,
        S,
        D,
        Space,
        LeftShift,
        LeftControl,
        Escape,
        Count
    };

    enum class MouseButton : u8
    {
        Left,
        Right,
        Middle,
        Count
    };

    inline constexpr std::size_t KEY_COUNT          = static_cast<std::size_t>(Key::Count);
    inline constexpr std::size_t MOUSE_BUTTON_COUNT = static_cast<std::size_t>(MouseButton::Count);

    struct FrameInput
    {
        std::bitset<KEY_COUNT>          KeysDown;
        std::bitset<MOUSE_BUTTON_COUNT> MouseButtonsDown;
        glm::vec2                       CursorPosition{}; // Screen coordinates
        glm::vec2                       CursorDelta{};    // Delta since previous PollEvents
        glm::vec2                       ScrollDelta{};    // Accumulated since previous PollEvents
    };
}
