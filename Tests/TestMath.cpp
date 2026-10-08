// Automatically generated based on project rules

#include "Graphics/Vulkan/VulkanGlobalUniforms.hpp"

#include "Math/Math.hpp"

#include "Vendor/doctest/doctest.hpp"

#include <array>
#include <cmath>
#include <cstddef>

namespace
{
    using Engine::Graphics::GlobalUniformData;

    namespace Math = Engine::Math;

    // ----- Type layout -----

    static_assert(sizeof(glm::vec2) == 8 && alignof(glm::vec2) == 4, "glm::vec2 is no longer tightly packed");
    static_assert(sizeof(glm::vec3) == 12 && alignof(glm::vec3) == 4,
                  "glm::vec3 is no longer tightly packed, the vertex layout depends on it");
    static_assert(sizeof(glm::vec4) == 16, "glm::vec4 is no longer tightly packed");
    static_assert(sizeof(glm::mat4) == 64, "glm::mat4 is no longer tightly packed");

    // ----- Uniform layout (std140) -----

    static_assert(offsetof(GlobalUniformData, Model) == 0 && offsetof(GlobalUniformData, View) == 64
                      && offsetof(GlobalUniformData, Projection) == 128 && sizeof(GlobalUniformData) == 192,
                  "GlobalUniformData is out of sync with the std140 block in Vert.glsl");

    // ----- World axes -----

    [[nodiscard]] constexpr Engine::f32 Dot(glm::vec3 a, glm::vec3 b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    [[nodiscard]] constexpr glm::vec3 Cross(glm::vec3 a, glm::vec3 b)
    {
        return { (a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x) };
    }

    // Viewer convention: Blender front view, looking along +Y
    static_assert(Math::WORLD_RIGHT.x == 1.0f && Math::WORLD_RIGHT.y == 0.0f && Math::WORLD_RIGHT.z == 0.0f,
                  "WORLD_RIGHT must be +X");
    static_assert(Math::WORLD_FORWARD.x == 0.0f && Math::WORLD_FORWARD.y == 1.0f && Math::WORLD_FORWARD.z == 0.0f,
                  "WORLD_FORWARD must be +Y");
    static_assert(Math::WORLD_UP.x == 0.0f && Math::WORLD_UP.y == 0.0f && Math::WORLD_UP.z == 1.0f,
                  "WORLD_UP must be +Z");

    static_assert(Dot(Math::WORLD_RIGHT, Math::WORLD_RIGHT) == 1.0f
                      && Dot(Math::WORLD_FORWARD, Math::WORLD_FORWARD) == 1.0f
                      && Dot(Math::WORLD_UP, Math::WORLD_UP) == 1.0f,
                  "World axes must be unit length");
    static_assert(Dot(Math::WORLD_RIGHT, Math::WORLD_FORWARD) == 0.0f && Dot(Math::WORLD_RIGHT, Math::WORLD_UP) == 0.0f
                      && Dot(Math::WORLD_FORWARD, Math::WORLD_UP) == 0.0f,
                  "World axes must be mutually orthogonal");

    // Right x Forward = Up keeps the viewer triple right-handed, so +X ends up on the right side of the screen
    static_assert(Cross(Math::WORLD_RIGHT, Math::WORLD_FORWARD).x == Math::WORLD_UP.x
                      && Cross(Math::WORLD_RIGHT, Math::WORLD_FORWARD).y == Math::WORLD_UP.y
                      && Cross(Math::WORLD_RIGHT, Math::WORLD_FORWARD).z == Math::WORLD_UP.z,
                  "World axes must form a right-handed viewer triple (right, forward, up)");

    // ----- Helpers -----

    [[nodiscard]] glm::vec4 TransformPoint(const glm::mat4& matrix, glm::vec3 point)
    {
        return matrix * glm::vec4(point, 1.0f);
    }

    [[nodiscard]] glm::vec4 TransformDirection(const glm::mat4& matrix, glm::vec3 direction)
    {
        return matrix * glm::vec4(direction, 0.0f);
    }

    [[nodiscard]] glm::vec3 ToNdc(const glm::vec4& clip)
    {
        return glm::vec3(clip) / clip.w;
    }

    // ----- Perspective -----

    struct ProjectionCase
    {
        const char* Name;
        Engine::f32 FovY;
        Engine::f32 Aspect;
        Engine::f32 Near;
        Engine::f32 Far;
    };

    const std::array<ProjectionCase, 3> PROJECTION_CASES = {
        ProjectionCase{ .Name   = "sandbox default",
                        .FovY   = glm::radians(45.0f),
                        .Aspect = 16.0f / 9.0f,
                        .Near   = 0.1f,
                        .Far    = 100.0f },
        ProjectionCase{
            .Name = "square and narrow", .FovY = glm::radians(30.0f), .Aspect = 1.0f, .Near = 1.0f, .Far = 10.0f },
        ProjectionCase{
            .Name = "portrait and wide", .FovY = glm::radians(90.0f), .Aspect = 0.5f, .Near = 0.01f, .Far = 1000.0f }
    };

    TEST_CASE("Perspective maps the near plane to depth 0 and the far plane to depth 1")
    {
        for (const ProjectionCase& testCase : PROJECTION_CASES)
        {
            INFO(testCase.Name);

            const glm::mat4 projection = Math::Perspective(testCase.FovY, testCase.Aspect, testCase.Near, testCase.Far);

            // Right-handed view space looks down -Z
            const glm::vec4 nearClip = TransformPoint(projection, { 0.0f, 0.0f, -testCase.Near });
            const glm::vec4 farClip  = TransformPoint(projection, { 0.0f, 0.0f, -testCase.Far });

            CHECK(nearClip.w == doctest::Approx(testCase.Near).epsilon(Math::EPSILON));
            CHECK(farClip.w == doctest::Approx(testCase.Far).epsilon(Math::EPSILON));
            CHECK(ToNdc(nearClip).z == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(ToNdc(farClip).z == doctest::Approx(1.0f).epsilon(Math::EPSILON));
        }
    }

    TEST_CASE("Perspective flips clip-space Y for Vulkan and keeps X")
    {
        for (const ProjectionCase& testCase : PROJECTION_CASES)
        {
            INFO(testCase.Name);

            const glm::mat4 projection = Math::Perspective(testCase.FovY, testCase.Aspect, testCase.Near, testCase.Far);

            const Engine::f32 distance = (testCase.Near + testCase.Far) * 0.5f;
            const glm::vec3   above    = ToNdc(TransformPoint(projection, { 0.0f, 1.0f, -distance }));
            const glm::vec3   right    = ToNdc(TransformPoint(projection, { 1.0f, 0.0f, -distance }));

            // Vulkan framebuffer Y points down, so view-space up lands in negative NDC Y
            CHECK(above.y < 0.0f);
            CHECK(above.x == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(right.x > 0.0f);
            CHECK(right.y == doctest::Approx(0.0f).epsilon(Math::EPSILON));
        }
    }

    TEST_CASE("Perspective spans the vertical field of view and scales X by the aspect ratio")
    {
        for (const ProjectionCase& testCase : PROJECTION_CASES)
        {
            INFO(testCase.Name);

            const glm::mat4 projection = Math::Perspective(testCase.FovY, testCase.Aspect, testCase.Near, testCase.Far);

            // Points on the top and right frustum edges
            const Engine::f32 distance   = (testCase.Near + testCase.Far) * 0.5f;
            const Engine::f32 halfHeight = distance * std::tan(testCase.FovY * 0.5f);
            const Engine::f32 halfWidth  = halfHeight * testCase.Aspect;

            const glm::vec3 topEdge   = ToNdc(TransformPoint(projection, { 0.0f, halfHeight, -distance }));
            const glm::vec3 rightEdge = ToNdc(TransformPoint(projection, { halfWidth, 0.0f, -distance }));

            CHECK(topEdge.y == doctest::Approx(-1.0f).epsilon(Math::EPSILON));
            CHECK(rightEdge.x == doctest::Approx(1.0f).epsilon(Math::EPSILON));
        }
    }

    // ----- LookAt -----

    struct ViewCase
    {
        const char* Name;
        glm::vec3   Eye;
        glm::vec3   Target;
    };

    const std::array<ViewCase, 5> VIEW_CASES = {
        ViewCase{ .Name = "sandbox camera", .Eye = { 0.0f, -30.0f, 10.0f }, .Target = { 0.0f, 0.0f, 0.0f } },
        ViewCase{ .Name = "rear view from above", .Eye = { 0.0f, 15.0f, 10.0f }, .Target = { 0.0f, 0.0f, 0.0f } },
        ViewCase{ .Name = "along world forward", .Eye = { 0.0f, 0.0f, 0.0f }, .Target = Math::WORLD_FORWARD * 5.0f },
        ViewCase{ .Name = "oblique off the origin", .Eye = { 3.0f, -4.0f, 2.0f }, .Target = { -1.0f, 2.0f, 0.5f } },
        ViewCase{ .Name = "steep but not vertical", .Eye = { 0.0f, 0.0f, 10.0f }, .Target = { 0.05f, 0.0f, 0.0f } }
    };

    TEST_CASE("LookAt moves the eye to the origin and the target onto the negative view Z axis")
    {
        for (const ViewCase& testCase : VIEW_CASES)
        {
            INFO(testCase.Name);

            const glm::mat4   view     = Math::LookAt(testCase.Eye, testCase.Target);
            const glm::vec4   eye      = TransformPoint(view, testCase.Eye);
            const glm::vec4   target   = TransformPoint(view, testCase.Target);
            const Engine::f32 distance = glm::length(testCase.Target - testCase.Eye);

            CHECK(eye.x == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(eye.y == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(eye.z == doctest::Approx(0.0f).epsilon(Math::EPSILON));

            CHECK(target.x == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(target.y == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(target.z == doctest::Approx(-distance).epsilon(Math::EPSILON));
        }
    }

    TEST_CASE("LookAt keeps world up in the upper half of the view")
    {
        for (const ViewCase& testCase : VIEW_CASES)
        {
            INFO(testCase.Name);

            const glm::vec4 up = TransformDirection(Math::LookAt(testCase.Eye, testCase.Target), Math::WORLD_UP);

            // World up never tilts sideways, it only leans towards or away from the viewer
            CHECK(up.x == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(up.y > 0.0f);
        }
    }

    TEST_CASE("LookAt produces a rigid right-handed transform")
    {
        for (const ViewCase& testCase : VIEW_CASES)
        {
            INFO(testCase.Name);

            const glm::mat3 rotation = glm::mat3(Math::LookAt(testCase.Eye, testCase.Target));

            CHECK(glm::length(rotation[0]) == doctest::Approx(1.0f).epsilon(Math::EPSILON));
            CHECK(glm::length(rotation[1]) == doctest::Approx(1.0f).epsilon(Math::EPSILON));
            CHECK(glm::length(rotation[2]) == doctest::Approx(1.0f).epsilon(Math::EPSILON));
            CHECK(glm::dot(rotation[0], rotation[1]) == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(glm::dot(rotation[0], rotation[2]) == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(glm::dot(rotation[1], rotation[2]) == doctest::Approx(0.0f).epsilon(Math::EPSILON));

            // Determinant of -1 would mirror the scene
            CHECK(glm::determinant(rotation) == doctest::Approx(1.0f).epsilon(Math::EPSILON));
        }
    }

    TEST_CASE("LookAt along world forward shows the Blender front view")
    {
        // Camera on the -Y side looking towards +Y; translating the eye must not change the orientation
        const std::array<glm::vec3, 2> eyes = { -Math::WORLD_FORWARD * 10.0f,
                                                (-Math::WORLD_FORWARD * 10.0f) + (Math::WORLD_RIGHT * 3.0f)
                                                    + (Math::WORLD_UP * 2.0f) };

        for (const glm::vec3& eye : eyes)
        {
            CAPTURE(eye.x);
            CAPTURE(eye.z);

            const glm::mat4 view = Math::LookAt(eye, eye + (Math::WORLD_FORWARD * 10.0f));

            // View space: +X right, +Y up, -Z into the screen
            const glm::vec4 right   = TransformDirection(view, Math::WORLD_RIGHT);
            const glm::vec4 up      = TransformDirection(view, Math::WORLD_UP);
            const glm::vec4 forward = TransformDirection(view, Math::WORLD_FORWARD);

            CHECK(right.x == doctest::Approx(1.0f).epsilon(Math::EPSILON));
            CHECK(right.y == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(right.z == doctest::Approx(0.0f).epsilon(Math::EPSILON));

            CHECK(up.x == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(up.y == doctest::Approx(1.0f).epsilon(Math::EPSILON));
            CHECK(up.z == doctest::Approx(0.0f).epsilon(Math::EPSILON));

            CHECK(forward.x == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(forward.y == doctest::Approx(0.0f).epsilon(Math::EPSILON));
            CHECK(forward.z == doctest::Approx(-1.0f).epsilon(Math::EPSILON));
        }
    }
}
