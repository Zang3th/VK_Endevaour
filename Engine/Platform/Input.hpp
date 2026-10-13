#pragma once

#include "Core/Types.hpp"

#include "Debug/Log.hpp"

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
        std::bitset<KEY_COUNT>          KeysDown;         // Which keys are currently pressed
        std::bitset<MOUSE_BUTTON_COUNT> MouseButtonsDown; // Which mouse buttons are currently pressed
        glm::vec2                       CursorPosition{}; // Screen coordinates
        glm::vec2                       CursorDelta{};    // Cursor delta since previous PollEvents
        glm::vec2                       ScrollDelta{};    // Scroll delta since previous PollEvents

        [[nodiscard]] b8 IsKeyDown(Key key) const
        {
            ASSERT(key != Key::Count, "IsKeyDown: Key::Count is not a key!");
            return KeysDown[(u8)key];
        }

        [[nodiscard]] b8 IsMouseButtonDown(MouseButton mouseButton) const
        {
            ASSERT(mouseButton != MouseButton::Count, "IsMouseButtonDown: MouseButton::Count is not a button!");
            return MouseButtonsDown[(u8)mouseButton];
        }
    };
}
