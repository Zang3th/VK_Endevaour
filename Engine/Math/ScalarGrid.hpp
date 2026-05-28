#pragma once

#include "Core/Types.hpp"

#include <vector>

namespace Engine::Math
{
    class ScalarGrid
    {
    public:
        ScalarGrid(u32 width, u32 height);

        [[nodiscard]] u32 Width() const;
        [[nodiscard]] u32 Height() const;
        [[nodiscard]] u32 Size() const;

        [[nodiscard]] float&       operator()(u32 x, u32 y);
        [[nodiscard]] const float& operator()(u32 x, u32 y) const;

        [[nodiscard]] const float* Data() const;

        void Fill(float value);
        void Resize(u32 width, u32 height);
        void Clear();

    private:
        u32                _width  = 0;
        u32                _height = 0;
        std::vector<float> _data;
    };
}
