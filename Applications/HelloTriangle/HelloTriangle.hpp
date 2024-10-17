#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>

#include "Core/Debug/Log.hpp"
#include "Lookup.hpp"

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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapChainProperties
{
    VkExtent2D extent;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
};

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
};

class HelloTriangle
{
    public:
        void Run();
        bool framebufferResized = false;

    private:
        GLFWwindow* _window;
        VkInstance  _instance;
        VkDebugUtilsMessengerEXT _debugMessenger;
        VkSurfaceKHR _surface;
        VkPhysicalDevice _physicalDevice;
        SwapChainSupport _swapChainSupport;
        QueueFamilyIndices _queueFamilyIndices;
        VkDevice _device;
        VkQueue _graphicsQueue, _presentQueue;
        SwapChainProperties _swapChainProperties;
        VkSwapchainKHR _swapChain;
        std::vector<VkImage> _swapChainImages;
        std::vector<VkImageView> _swapChainImageViews;
        VkRenderPass _renderPass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;
        std::vector<VkFramebuffer> _swapChainFramebuffers;
        VkCommandPool _commandPool;
        std::vector<VkCommandBuffer> _commandBuffers;
        std::vector<VkSemaphore> _imageAvailableSemaphores;
        std::vector<VkSemaphore> _renderFinishedSemaphores;
        std::vector<VkFence> _inFlightFences;
        uint32_t _currentFrame = 0;

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
        bool IsDeviceSuitable(VkPhysicalDevice device);
        void PickPhysicalDevice();
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupport QuerySwapChainSupport(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        void CreateLogicalDevice();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        void CreateGraphicsPipeline();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void RecordCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void CreateSyncObjects();
        void DrawFrame();
        void CleanupSwapChain();
        void RecreateSwapChain();
};
