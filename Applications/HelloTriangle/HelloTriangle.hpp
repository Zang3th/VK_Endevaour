#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <unordered_map>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Map depth range from 0.0 to 1.0 (instead of OpenGL's -1.0 to 1.0)
#include "Vendor/glm/gtc/matrix_transform.hpp"
#include "Core/Debug/Log.hpp"
#include "Lookup.hpp"
#include "Vertex.hpp"
#include "Vendor/stb_image/stb_image.hpp"
#include "Vendor/tinyobjloader/tiny_obj_loader.hpp"

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

const uint32_t WIDTH = 1200;
const uint32_t HEIGHT = 1200;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "Models/viking_room.obj";
const std::string TEXTURE_PATH = "Textures/viking_room.png";

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
    public:
        SwapChainProperties _swapChainProperties;
    private:
        VkSwapchainKHR _swapChain;
        std::vector<VkImage> _swapChainImages;
        std::vector<VkImageView> _swapChainImageViews;
        VkRenderPass _renderPass;
        VkDescriptorSetLayout _descriptorSetLayout;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;
        std::vector<VkFramebuffer> _swapChainFramebuffers;
        VkCommandPool _commandPool;
        std::vector<VkCommandBuffer> _commandBuffers;
        std::vector<VkSemaphore> _imageAvailableSemaphores;
        std::vector<VkSemaphore> _renderFinishedSemaphores;
        std::vector<VkFence> _inFlightFences;
        uint32_t _currentFrame = 0;

        std::vector<Vertex> _vertices;
        std::vector<uint32_t> _indices;
        VkBuffer _vertexBuffer;
        VkDeviceMemory _vertexBufferMemory;
        VkBuffer _indexBuffer;
        VkDeviceMemory _indexBufferMemory;

        std::vector<VkBuffer> _uniformBuffers;
        std::vector<VkDeviceMemory> _uniformBuffersMemory;
        std::vector<void*> _uniformBuffersMapped;
        VkDescriptorPool _descriptorPool;
        std::vector<VkDescriptorSet> _descriptorSets;
        uint32_t _mipLevels;
        VkImage _textureImage;
        VkDeviceMemory _textureImageMemory;
        VkImageView _textureImageView;
        VkSampler _textureSampler;
        VkImage _depthImage;
        VkDeviceMemory _depthImageMemory;
        VkImageView _depthImageView;
        VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        VkImage _colorImage;
        VkDeviceMemory _colorImageMemory;
        VkImageView _colorImageView;

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
        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
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
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateVertexBuffer();
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CreateIndexBuffer();
        void CreateDescriptorSetLayout();
        void CreateUniformBuffers();
        void UpdateUniformBuffer(uint32_t frame);
        void CreateDescriptorPool();
        void CreateDescriptorSets();
        void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
        void CreateTextureImage(const std::string& filepath);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CreateTextureImageView();
        void CreateTextureSampler();
        void CreateDepthResources();
        void LoadModel(const std::string& filepath);
        void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        VkSampleCountFlagBits GetMSAASampleCount();
        void CreateColorResources();
};
