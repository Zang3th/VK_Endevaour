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

        // Amount of frames that got rendered
        u64 FrameCounter = 0;
    };

    class Timer
    {
    public:
        Timer();

        Timer(const Timer&)            = delete;
        Timer& operator=(const Timer&) = delete;

        [[nodiscard]] const FrameTiming& GetFrameTiming() const { return m_FrameTiming; }
        [[nodiscard]] std::string        GetEngineTotalRuntimeString() const;
        [[nodiscard]] std::string        GetEngineFPSAverageString() const;

        // Syncs clock and delta timings to current frame
        // Internally resets last clock and last delta timings to compensate for not rendering a while
        void SyncFrame();

        // Increments the timer and calculates frame timings
        void Tick();

    private:
        void Reset();

        using Clock = std::chrono::high_resolution_clock;

        Clock::time_point m_StartClock;
        Clock::time_point m_LastClock;

        // Accumulates delta time to calculate the fps once per second
        f64 m_FPSAccumulatedMilliseconds = 0.0;

        // Loop counter to calculate fps
        u64 m_FPSLoopCounter = 0;

        // Struct with main rendering timings (for animation, visualization etc.)
        FrameTiming m_FrameTiming;
    };
}
