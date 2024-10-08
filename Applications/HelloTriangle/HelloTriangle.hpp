#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Core/Debug/Log.hpp"
#include "Lookup.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

#define VK_VERIFY_RESULT(func) \
{ \
    VkResult res = (func); \
    if(res != VK_SUCCESS) \
    { \
        LOG_ERROR("[VkResult] {} in\n\t\t\t{}:{}", VkResultToString(res), __FILE__, __LINE__); \
        __builtin_debugtrap(); \
    } \
}

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
};

class HelloTriangle
{
    public:
        void Run();

    private:
        GLFWwindow* _window;
        VkInstance  _instance;
        VkDebugUtilsMessengerEXT _debugMessenger;
        VkSurfaceKHR _surface;
        VkPhysicalDevice _physicalDevice;
        QueueFamilyIndices _queueFamilyIndices;
        VkDevice _device;
        VkQueue _graphicsQueue, _presentQueue;

        void InitWindow();
        void InitVulkan();
        void MainLoop();
        void CleanUp();
        void CreateInstance();
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void SetupDebugMessenger();
        std::vector<const char*> GetRequiredExtensions();
        bool CheckValidationLayerSupport();
        void CreateSurface();
        void PickPhysicalDevice();
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        void CreateLogicalDevice();
};
