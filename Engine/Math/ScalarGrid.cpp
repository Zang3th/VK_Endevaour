#include "ScalarGrid.hpp"

#include "Debug/Log.hpp"

namespace Engine::Math
{
    ScalarGrid::ScalarGrid(u32 width, u32 height) : _width(width), _height(height)
    {
        _data.resize((size_t)_width * _height);
    }

    [[nodiscard]] u32 ScalarGrid::Width() const
    {
        return _width;
    }

    [[nodiscard]] u32 ScalarGrid::Height() const
    {
        return _height;
    }

    [[nodiscard]] u32 ScalarGrid::Size() const
    {
        return (u32)_data.size();
    }

    [[nodiscard]] float& ScalarGrid::operator()(u32 x, u32 y)
    {
        ASSERT(x < _width, "ScalarGrid: x = {} exceeds grid width = {}", x, _width);
        ASSERT(y < _height, "ScalarGrid: y = {} exceeds grid height = {}", y, _height);
        return _data[(x * _height) + y];
    }

    [[nodiscard]] const float& ScalarGrid::operator()(u32 x, u32 y) const
    {
        ASSERT(x < _width, "ScalarGrid: x = {} exceeds grid width = {}", x, _width);
        ASSERT(y < _height, "ScalarGrid: y = {} exceeds grid height = {}", y, _height);
        return _data[(x * _height) + y];
    }

    [[nodiscard]] const float* ScalarGrid::Data() const
    {
        return _data.data();
    }

    void ScalarGrid::Fill(float value)
    {
        for (float& index : _data)
        {
            index = value;
        }
    }

    void ScalarGrid::Resize(u32 width, u32 height)
    {
        _data.resize((size_t)width * height);
        _width  = width;
        _height = height;
    }

    void ScalarGrid::Clear()
    {
        _data.clear();
        _width  = 0;
        _height = 0;
    }
}
