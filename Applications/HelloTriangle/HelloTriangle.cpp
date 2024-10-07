#include "HelloTriangle.hpp"

// ----- Current translation unit

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData
)
{
    switch(messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: LOG_VERBOSE("[VkMsg | {}]: {}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    LOG_INFO   ("[VkMsg | {}]: {}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: LOG_WARN   ("[VkMsg | {}]: {}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   LOG_ERROR  ("[VkMsg | {}]: {}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        default: LOG_ERROR("Caught unknown severity in vulkan debug callback!"); break;
    }

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT
(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT
(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

// ----- Public -----

void HelloTriangle::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
}

// ----- Private -----

void HelloTriangle::InitWindow()
{
    ASSERT(glfwInit(), "Failed to initialize GLFW: {}", glfwGetError(nullptr));
    LOG_INFO("Initialized GLFW!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    _window = glfwCreateWindow(WIDTH, HEIGHT, "HelloTriangle", nullptr, nullptr);
    ASSERT(_window, "Failed to create GLFW window: {}", glfwGetError(nullptr));
    LOG_INFO("Created GLFW window!");
}

void HelloTriangle::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    PickPhysicalDevice();
    CreateLogicalDevice();
}

void HelloTriangle::MainLoop()
{
    while(!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
    }
}

void HelloTriangle::CleanUp()
{
    if(enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }

    vkDestroyDevice(_device, nullptr);
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void HelloTriangle::CreateInstance()
{
    if(enableValidationLayers)
    {
        ASSERT(CheckValidationLayerSupport(), "Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if(enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VK_VERIFY_RESULT(vkCreateInstance(&createInfo, nullptr, &_instance));
    LOG_INFO("Created Vulkan instance!");
}

void HelloTriangle::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr;

}

void HelloTriangle::SetupDebugMessenger()
{
    if(!enableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);

    VK_VERIFY_RESULT(CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger));
    LOG_INFO("Setup debug messenger!");
}

std::vector<const char*> HelloTriangle::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if(enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool HelloTriangle::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for(const auto& layerProperties : availableLayers)
        {
            if(strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if(!layerFound)
        {
            return false;
        }
    }

    return true;
}

void HelloTriangle::PickPhysicalDevice()
{
    // Query for devices
    uint32_t deviceCount = 0;
    VK_VERIFY_RESULT(vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr));
    ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan support!");

    // Save device handles
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_VERIFY_RESULT(vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data()));

    // Check requirements
    VkPhysicalDeviceProperties deviceProperties;
    for(const auto& device : devices)
    {
        // Get first discrete device and then exit
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            _physicalDevice = device;
            break;
        }
    }

    ASSERT(_physicalDevice, "Failed to find discrete GPU!");

    // Query device features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures);
    LOG_INFO("GPU: {} ({}), Driver: {}", deviceProperties.deviceName, VkVendorIDToString(deviceProperties.vendorID), VkDriverVersionToString(deviceProperties.driverVersion));

    // Queue family indices
    _queueFamilyIndices = FindQueueFamilies(_physicalDevice);
    LOG_INFO("Found graphics capable queue family (index {})", _queueFamilyIndices.graphicsFamily);
}

QueueFamilyIndices HelloTriangle::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    // Query for queue family count and properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // Query queue family for graphics bit
    int i = 0;
    bool success = false;
    for(const auto& queueFamily : queueFamilies)
    {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            success = true;
            break;
        }
        i++;
    }
    ASSERT(success, "Found no graphics capable queue family!");

    return indices;
}

void HelloTriangle::CreateLogicalDevice()
{
    // Create queue with graphics capabilties
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = _queueFamilyIndices.graphicsFamily;
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Define features you want to use (e.g. geometry shaders)
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0; // Device specific validation layers get ignored in newer vulkan versions

    VK_VERIFY_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));
    LOG_INFO("Created device!");

    // Retrieve queue handle
    vkGetDeviceQueue(_device, _queueFamilyIndices.graphicsFamily, 0, &_graphicsQueue);
}
