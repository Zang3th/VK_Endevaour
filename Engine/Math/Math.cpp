#include "Math.hpp"

#include "Debug/Log.hpp"

#include "Vendor/glm/ext/matrix_clip_space.hpp"
#include "Vendor/glm/gtx/vector_query.hpp"

namespace Engine::Math
{
    [[nodiscard]] glm::mat4 Perspective(f32 fovY, f32 aspect, f32 zNear, f32 zFar)
    {
        glm::mat4 persp = glm::perspectiveRH_ZO(fovY, aspect, zNear, zFar);
        persp[1][1] *= -1; // Flip Y-Coordinate of clip coordinates because glm follows OpenGL's convention that +Y is
                           // up, this affects the winding order
        return persp;
    }

    [[nodiscard]] glm::mat4 LookAt(const glm::vec3& eye, const glm::vec3& target)
    {
        ASSERT(eye != target, "Eye and target vector are identical!");
        const glm::vec3 viewDir = glm::normalize(eye - target);
        ASSERT(!glm::areCollinear(viewDir, WORLD_UP, EPSILON), "View direction and world up vector are collinear!");
        return glm::lookAt(eye, target, WORLD_UP);
    }
}
