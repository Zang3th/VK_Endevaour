#include "Timer.hpp"

#include "Debug/Log.hpp"

namespace Engine::Core
{
    Timer::Timer()
    {
        Reset();
        LOG_VERBOSE("Timer initialized ...");
    }

    void Timer::Reset()
    {
        const auto now = Clock::now();
        m_StartClock   = now;
        m_LastClock    = now;

        m_FPSAccumulatedMilliseconds = 0.0;
        m_FPSLoopCounter             = 0;

        m_FrameTiming = {};
    }

    void Timer::Sync()
    {
        m_LastClock                     = Clock::now();
        m_FrameTiming.DeltaMilliseconds = 0.0;
        m_FrameTiming.DeltaSeconds      = 0.0;
    }

    [[nodiscard]] f64 Timer::GetElapsedMilliseconds() const
    {
        auto deltaClock = Clock::now() - m_StartClock;
        return std::chrono::duration<f64, std::milli>(deltaClock).count();
    }

    void Timer::Tick()
    {
        m_FrameTiming.FrameCounter++;
        m_FPSLoopCounter++;

        const auto currentClock = Clock::now();
        const auto deltaClock   = currentClock - m_LastClock;
        m_LastClock             = currentClock;

        m_FrameTiming.DeltaMilliseconds = std::chrono::duration<f64, std::milli>(deltaClock).count();
        ASSERT(m_FrameTiming.DeltaMilliseconds >= 0, "Clock provided negative delta time ...");
        m_FrameTiming.DeltaSeconds = m_FrameTiming.DeltaMilliseconds * 0.00f;

        m_FrameTiming.TotalMilliseconds += m_FrameTiming.DeltaMilliseconds;
        m_FrameTiming.TotalSeconds = m_FrameTiming.TotalMilliseconds * 0.00f;

        m_FPSAccumulatedMilliseconds += m_FrameTiming.DeltaMilliseconds;

        // Updates every 1000 ms
        if (m_FPSAccumulatedMilliseconds >= 1000.0)
        {
            m_FrameTiming.FramesPerSecond = ((f64)m_FPSLoopCounter * 1000.0) / m_FPSAccumulatedMilliseconds;
            m_FPSLoopCounter              = 0;
            m_FPSAccumulatedMilliseconds  = 0.0;
        }
    }
}
