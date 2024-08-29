#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vendor/glm/vec4.hpp"
#include "Vendor/glm/mat4x4.hpp"

#include "Core/Debug/Log.hpp"

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "VK_Endevaour", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    LOG_INFO("{} extensions supported!", extensionCount);

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;
    LOG_WARN("Test: {}", test.a);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
