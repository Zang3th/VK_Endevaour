#include "ProfilerPanel.hpp"

#include "Vendor/imgui/imgui.h"

namespace Engine::Graphics
{
    // ----- Public -----

    ProfilerPanel::ProfilerPanel() {}

    ProfilerPanel::~ProfilerPanel() {}

    void ProfilerPanel::Render(const Core::FrameTiming& timing)
    {
        constexpr f64 sampleInterval = 50; // Sample every 50 ms

        if (timing.TotalMilliseconds - m_LastHistorySample >= sampleInterval)
        {
            m_FrameTimeHistory[m_HistoryOffset] = (f32)timing.DeltaMilliseconds;
            m_HistoryOffset                     = (m_HistoryOffset + 1) % HistorySize;
            m_LastHistorySample                 = timing.TotalMilliseconds;
        }

        if (!ImGui::Begin("Profiler"))
        {
            ImGui::End();
            return;
        }

        // --- Timing
        ImGui::SeparatorText("Timing");
        ImGui::Text("%.2f FPS (%.2f ms/frame)", timing.FramesPerSecond, timing.DeltaMilliseconds);

        ImGui::Separator();
        ImGui::PlotLines("##FrameTime",
                         m_FrameTimeHistory.data(),
                         (i32)m_FrameTimeHistory.size(),
                         (i32)m_HistoryOffset,
                         "Frame time",
                         0.0f,
                         16.67,
                         ImVec2{ -1.0f, 100.0f });
        ImGui::Separator();

        ImGui::Text("%-13s %8.2f ms (Frame %llu)",
                    "Lowest  delta",
                    timing.Benchmark.LowestDeltaMilliseconds,
                    (ull)timing.Benchmark.LowestDeltaFrame);
        ImGui::Text("%-13s %8.2f ms (Frame %llu)",
                    "Highest delta",
                    timing.Benchmark.HighestDeltaMilliseconds,
                    (ull)timing.Benchmark.HighestDeltaFrame);

        // --- Stats
        ImGui::SeparatorText("Stats");
        ImGui::Text("Frame   %llu", (ull)timing.FrameCounter);
        ImGui::Text("Runtime %.2f s", timing.TotalSeconds);

        ImGui::End();
    }

    // ----- Private -----

    void ProfilerPanel::Init() {}
}
