#pragma once

namespace Engine::Graphics
{
    struct RenderSettings;
}

namespace Engine::Graphics
{
    class MainMenuBar
    {
    public:
        MainMenuBar()  = default;
        ~MainMenuBar() = default;

        MainMenuBar(const MainMenuBar&)            = delete;
        MainMenuBar& operator=(const MainMenuBar&) = delete;

        void Render(RenderSettings& renderSettings);
    };
}
