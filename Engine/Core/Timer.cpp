#include "Timer.hpp"

#include "Core/Utility.hpp"

#include "Debug/Log.hpp"

namespace Engine::Core
{
    // ----- Public -----

    Timer::Timer()
    {
        Reset();
        LOG_INFO("Timer initialized ...");
    }

    [[nodiscard]] std::string Timer::GetEngineTotalRuntimeString() const
    {
        auto deltaClock = Clock::now() - m_StartClock;
        return Utility::MillisecondsToString(std::chrono::duration<f64, std::milli>(deltaClock).count());
    }

    [[nodiscard]] std::string Timer::GetEngineFPSAverageString() const
    {
        return Utility::FPSToString((f64)m_FrameTiming.FrameCounter / m_FrameTiming.TotalSeconds);
    }

    void Timer::SyncFrame()
    {
        m_LastClock                     = Clock::now();
        m_FrameTiming.DeltaMilliseconds = 0.0;
        m_FrameTiming.DeltaSeconds      = 0.0;
    }

    void Timer::Tick()
    {
        // Increment counters
        m_FrameTiming.FrameCounter++;
        m_FPSLoopCounter++;

        // Get current clock and calculate delta clock
        const auto currentClock = Clock::now();
        const auto deltaClock   = currentClock - m_LastClock;
        m_LastClock             = currentClock;

        // Get delta time from clock in ms and secs
        m_FrameTiming.DeltaMilliseconds = std::chrono::duration<f64, std::milli>(deltaClock).count();
        ASSERT(m_FrameTiming.DeltaMilliseconds >= 0, "Clock provided negative delta time ...");
        m_FrameTiming.DeltaSeconds = m_FrameTiming.DeltaMilliseconds * 0.001f;

        // Add up total time
        m_FrameTiming.TotalMilliseconds += m_FrameTiming.DeltaMilliseconds;
        m_FrameTiming.TotalSeconds = m_FrameTiming.TotalMilliseconds * 0.001f;
        m_FPSAccumulatedMilliseconds += m_FrameTiming.DeltaMilliseconds;

        // Updates every 1000 ms
        if (m_FPSAccumulatedMilliseconds >= 1000.0)
        {
            // Calculate fps based on the amount of loops and the time it took
            m_FrameTiming.FramesPerSecond = ((f64)m_FPSLoopCounter * 1000.0) / m_FPSAccumulatedMilliseconds;

            // Reset fps related variables
            m_FPSLoopCounter             = 0;
            m_FPSAccumulatedMilliseconds = 0.0;
        }
    }

    // ----- Private -----

    void Timer::Reset()
    {
        const auto now = Clock::now();
        m_StartClock   = now;
        m_LastClock    = now;

        m_FPSAccumulatedMilliseconds = 0.0;
        m_FPSLoopCounter             = 0;

        m_FrameTiming = {};
    }
}
