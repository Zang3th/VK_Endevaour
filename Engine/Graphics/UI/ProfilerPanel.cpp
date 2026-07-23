#include "ProfilerPanel.hpp"

#include "Platform/Window.hpp"

#include "Vendor/imgui/imgui.h"

namespace Engine::Graphics
{
    // ----- Public -----

    void ProfilerPanel::Render(const Core::FrameTiming& timing, const Graphics::RenderStats& renderStats)
    {
        // Panel position
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2         position{ viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y };
        ImGui::SetNextWindowPos(position, ImGuiCond_Always, ImVec2{ 1.0f, 0.0f });

        // Panel size
        constexpr f32 panelWidth = 320.0f;
        ImGui::SetNextWindowSize(ImVec2{ panelWidth, 0.0f }, ImGuiCond_Always);

        // Panel flags
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                                                 | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
                                                 | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse
                                                 | ImGuiWindowFlags_NoSavedSettings;

        if (!ImGui::Begin("Profiler", nullptr, windowFlags))
        {
            ImGui::End();
            return;
        }

        constexpr f64 sampleInterval = 50; // Sample every 50 ms

        if (timing.TotalMilliseconds - m_LastHistorySample >= sampleInterval)
        {
            m_FrameTimeHistory[m_HistoryOffset] = (f32)timing.DeltaMilliseconds;
            m_HistoryOffset                     = (m_HistoryOffset + 1) % HistorySize;
            m_LastHistorySample                 = timing.TotalMilliseconds;
        }

        // Application
        ImGui::SeparatorText("Application");
        ImGui::Text("%-8s %3.1f s", "Runtime", timing.TotalSeconds);
        ImGui::Text("%-8s %4llu", "Frames", (ull)timing.FrameCounter);
        ImGui::Text("%-8s %4d x %-4d", "Window", Platform::Window::GetWidth(), Platform::Window::GetHeight());
        ImGui::NewLine();

        // Timing
        ImGui::SeparatorText("Timing");
        ImGui::Text("%4.2f FPS (%2.2f ms/frame)", timing.FramesPerSecond, timing.DeltaMilliseconds);

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

        ImGui::Text("%-11s %1.5f ms (Frame %4llu)",
                    "Lowest dt",
                    timing.Benchmark.LowestDeltaMilliseconds,
                    (ull)timing.Benchmark.LowestDeltaFrame);
        ImGui::Text("%-11s %4.2f ms (Frame %4llu)",
                    "Highest dt",
                    timing.Benchmark.HighestDeltaMilliseconds,
                    (ull)timing.Benchmark.HighestDeltaFrame);
        ImGui::NewLine();

        // Draw Stats
        ImGui::SeparatorText("Draw Stats");
        ImGui::Text("%-8s %2d", "Draws", renderStats.DrawCalls);
        ImGui::Text("%-8s %2d", "Models", renderStats.Models);
        ImGui::Text("%-8s %2d", "Vertices", renderStats.Vertices);
        ImGui::Text("%-8s %2d", "Indices", renderStats.Indices);

        ImGui::End();
    }
}
