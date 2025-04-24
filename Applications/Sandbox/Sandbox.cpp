#include "Sandbox.hpp"

#include <Core/Window.hpp>

void Sandbox::Run()
{
    Engine::Window::Init
    ({
        .Title = "Sandbox"
    });

    while(!Engine::Window::ShouldClose())
    {
        Engine::Window::PollEvents();
    }
}
