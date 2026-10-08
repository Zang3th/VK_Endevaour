#pragma once

#ifdef GLM_SETUP_INCLUDED
#error "GLM must only be included through Math/Math.hpp"
#endif

#include "Core/Types.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "Vendor/glm/ext/matrix_transform.hpp"
#include "Vendor/glm/gtx/hash.hpp"
#include "Vendor/glm/mat4x4.hpp"
#include "Vendor/glm/trigonometric.hpp"
#include "Vendor/glm/vec2.hpp"
#include "Vendor/glm/vec3.hpp"

namespace Engine::Math
{
    // World-Axes: Right-handed, Z-up (like Blender)
    inline constexpr glm::vec3 WORLD_RIGHT   = { 1.0f, 0.0f, 0.0f };
    inline constexpr glm::vec3 WORLD_FORWARD = { 0.0f, 1.0f, 0.0f };
    inline constexpr glm::vec3 WORLD_UP      = { 0.0f, 0.0f, 1.0f };

    inline constexpr f32 EPSILON = 1.0e-4f;

    [[nodiscard]] glm::mat4 Perspective(f32 fovY, f32 aspect, f32 zNear, f32 zFar);
    [[nodiscard]] glm::mat4 LookAt(const glm::vec3& eye, const glm::vec3& target);
}
