#include "Vendor/doctest/doctest.hpp"

#include "Math/ScalarGrid.hpp"

namespace
{
    TEST_CASE("ScalarGrid construction initializes dimensions and size")
    {
        Engine::Math::ScalarGrid grid{ 4, 3 };

        CHECK(grid.Width() == 4);
        CHECK(grid.Height() == 3);
        CHECK(grid.Size() == 12);
        CHECK(grid.Data() != nullptr);
    }

    TEST_CASE("ScalarGrid supports 1x1 grid")
    {
        Engine::Math::ScalarGrid grid{ 1, 1 };

        CHECK(grid.Width() == 1);
        CHECK(grid.Height() == 1);
        CHECK(grid.Size() == 1);

        grid(0, 0) = 42.0f;
        CHECK(grid(0, 0) == doctest::Approx(42.0f));
    }

    TEST_CASE("ScalarGrid stores independent values at all coordinates")
    {
        Engine::Math::ScalarGrid grid{ 8, 6 };

        for(Engine::u32 y = 0; y < grid.Height(); ++y)
        {
            for(Engine::u32 x = 0; x < grid.Width(); ++x)
            {
                grid(x, y) = static_cast<float>(x + (y * 100));
            }
        }

        for(Engine::u32 y = 0; y < grid.Height(); ++y)
        {
            for(Engine::u32 x = 0; x < grid.Width(); ++x)
            {
                CHECK(grid(x, y) == doctest::Approx(static_cast<float>(x + (y * 100))));
            }
        }
    }

    TEST_CASE("ScalarGrid supports boundary coordinates")
    {
        Engine::Math::ScalarGrid grid{ 10, 20 };

        grid(0, 0)  = 1.0f;
        grid(9, 0)  = 2.0f;
        grid(0, 19) = 3.0f;
        grid(9, 19) = 4.0f;

        CHECK(grid(0, 0) == doctest::Approx(1.0f));
        CHECK(grid(9, 0) == doctest::Approx(2.0f));
        CHECK(grid(0, 19) == doctest::Approx(3.0f));
        CHECK(grid(9, 19) == doctest::Approx(4.0f));
    }

    TEST_CASE("ScalarGrid::Fill overwrites every element")
    {
        Engine::Math::ScalarGrid grid{ 7, 5 };

        for(Engine::u32 y = 0; y < grid.Height(); ++y)
        {
            for(Engine::u32 x = 0; x < grid.Width(); ++x)
            {
                grid(x, y) = static_cast<float>(x + y);
            }
        }

        grid.Fill(-3.5f);

        for(Engine::u32 y = 0; y < grid.Height(); ++y)
        {
            for(Engine::u32 x = 0; x < grid.Width(); ++x)
            {
                CHECK(grid(x, y) == doctest::Approx(-3.5f));
            }
        }
    }

    TEST_CASE("ScalarGrid::Resize grows grid and updates dimensions")
    {
        Engine::Math::ScalarGrid grid{ 2, 2 };

        grid.Resize(5, 4);

        CHECK(grid.Width() == 5);
        CHECK(grid.Height() == 4);
        CHECK(grid.Size() == 20);
        CHECK(grid.Data() != nullptr);

        grid(4, 3) = 123.0f;
        CHECK(grid(4, 3) == doctest::Approx(123.0f));
    }

    TEST_CASE("ScalarGrid::Resize shrinks grid and updates dimensions")
    {
        Engine::Math::ScalarGrid grid{ 8, 8 };

        grid.Resize(2, 3);

        CHECK(grid.Width() == 2);
        CHECK(grid.Height() == 3);
        CHECK(grid.Size() == 6);

        grid(1, 2) = 77.0f;
        CHECK(grid(1, 2) == doctest::Approx(77.0f));
    }

    TEST_CASE("ScalarGrid::Resize to zero width produces empty grid")
    {
        Engine::Math::ScalarGrid grid{ 4, 4 };

        grid.Resize(0, 4);

        CHECK(grid.Width() == 0);
        CHECK(grid.Height() == 4);
        CHECK(grid.Size() == 0);
    }

    TEST_CASE("ScalarGrid::Resize to zero height produces empty grid")
    {
        Engine::Math::ScalarGrid grid{ 4, 4 };

        grid.Resize(4, 0);

        CHECK(grid.Width() == 4);
        CHECK(grid.Height() == 0);
        CHECK(grid.Size() == 0);
    }

    TEST_CASE("ScalarGrid::Resize to zero by zero produces empty grid")
    {
        Engine::Math::ScalarGrid grid{ 4, 4 };

        grid.Resize(0, 0);

        CHECK(grid.Width() == 0);
        CHECK(grid.Height() == 0);
        CHECK(grid.Size() == 0);
    }

    TEST_CASE("ScalarGrid::Clear resets observable state")
    {
        Engine::Math::ScalarGrid grid{ 5, 5 };

        grid.Fill(1.0f);
        grid.Clear();

        CHECK(grid.Width() == 0);
        CHECK(grid.Height() == 0);
        CHECK(grid.Size() == 0);
    }

    TEST_CASE("ScalarGrid::Data exposes contiguous readable storage")
    {
        Engine::Math::ScalarGrid grid{ 4, 3 };

        grid.Fill(6.0f);

        const float* data = grid.Data();

        REQUIRE(data != nullptr);

        for(Engine::u32 i = 0; i < grid.Size(); ++i)
        {
            CHECK(data[i] == doctest::Approx(6.0f));
        }
    }
}
