#pragma once

#include "Core/Timer.hpp"

#include <array>

namespace Engine::Graphics
{
    class ProfilerPanel
    {
    public:
        ProfilerPanel();
        ~ProfilerPanel();

        ProfilerPanel(const ProfilerPanel&)            = delete;
        ProfilerPanel& operator=(const ProfilerPanel&) = delete;

        void Render(const Core::FrameTiming& frameTiming);

    private:
        static constexpr u8 HistorySize = 240;

        std::array<f32, HistorySize> m_FrameTimeHistory{};
        u8                           m_HistoryOffset     = 0;
        f64                          m_LastHistorySample = 0.0;

        void Init();
    };
}
