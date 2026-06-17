#pragma once

#include "Core/Types.hpp"

#include <chrono>

namespace Engine::Core
{
    struct FrameTiming
    {
        f64 DeltaSeconds      = 0.0;
        f64 DeltaMilliseconds = 0.0;

        f64 TotalSeconds      = 0.0;
        f64 TotalMilliseconds = 0.0;

        f64 FramesPerSecond = 0.0;
        u64 FrameCounter    = 0;
    };

    class Timer
    {
    public:
        Timer();

        Timer(const Timer&)            = delete;
        Timer& operator=(const Timer&) = delete;

        void Reset();
        void Tick();
        void Sync();

        [[nodiscard]] const FrameTiming& GetFrameTiming() const { return m_FrameTiming; }
        [[nodiscard]] f64                GetElapsedMilliseconds() const;

    private:
        using Clock = std::chrono::high_resolution_clock;

        Clock::time_point m_StartClock;
        Clock::time_point m_LastClock;

        f64 m_FPSAccumulatedMilliseconds = 0.0;
        u64 m_FPSLoopCounter             = 0;

        FrameTiming m_FrameTiming;
    };
}
