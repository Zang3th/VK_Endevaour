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
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
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

    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
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

void HelloTriangle::CreateSurface()
{
    VK_VERIFY_RESULT(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface));
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

    // Check each device for suitability
    for(const auto& device : devices)
    {
        if(IsDeviceSuitable(device))
        {
            _physicalDevice = device;
            break;
        }
    }

    ASSERT(_physicalDevice, "Failed to find suitable device!");

    // TODO: Move this also into the IsDeviceSuitable()-Function
    _queueFamilyIndices = FindQueueFamilies(_physicalDevice);
    LOG_INFO("Found graphics capable queue family (index {})", _queueFamilyIndices.graphicsFamily);
    LOG_INFO("Found surface presentation queue family (index {})", _queueFamilyIndices.presentFamily);
}

bool HelloTriangle::IsDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Check requirements (discrete GPU and support for swap chain)
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && CheckDeviceExtensionSupport(device))
    {
        LOG_INFO("Found discrete GPU with swap chain support!");
        _swapChainSupport = QuerySwapChainSupport(device);

        if(!_swapChainSupport.formats.empty() && !_swapChainSupport.presentModes.empty())
        {
            LOG_INFO("Found adequate swap chain support!");

            // Query and print device features
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            LOG_INFO("GPU: {} ({}), Driver: {}", deviceProperties.deviceName, VkVendorIDToString(deviceProperties.vendorID), VkDriverVersionToString(deviceProperties.driverVersion));

            return true;
        }
    }

    return false;
}

VkSurfaceFormatKHR HelloTriangle::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // Check if prefered format is available
    for(const auto& availableFormat : availableFormats)
    {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // If that's not available return the next best thing
    return availableFormats[0];
}

VkPresentModeKHR HelloTriangle::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    // Check if mailbox mode aka triple buffering is available
    for(const auto& availablePresentMode : availablePresentModes)
    {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    // Queue mode is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangle::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites)
{
    // Check for special case
    if(capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilites.currentExtent;
    }
    else
    {
        // Get window size in pixels
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Clamp width and height between allowed min and max values of the display manager implementation
        actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);

        return actualExtent;
    }
}

bool HelloTriangle::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    // Get number of extensions
    uint32_t extensionCount;
    VK_VERIFY_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

    // Get extensions
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    VK_VERIFY_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

    // Create set out of needed extensions
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // Iterate over all available extensions and check if the required ones are amongst thme
    for(const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    // Return false if not all requested extensions are available
    return requiredExtensions.empty();
}

// PERF: This returns a struct with several vectors inside. Optimize later.
SwapChainSupport HelloTriangle::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupport swapChainSupport{};

    // Query for basic surface capabilites
    VK_VERIFY_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &swapChainSupport.capabilities));

    // Query surface format count
    uint32_t formatCount = 0;
    VK_VERIFY_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr));
    ASSERT(formatCount > 0, "Found no supported surface formats!");

    // Query surface formats
    swapChainSupport.formats.resize(formatCount);
    VK_VERIFY_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, swapChainSupport.formats.data()));

    // Query presentation mode count
    uint32_t presentModeCount = 0;
    VK_VERIFY_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr));
    ASSERT(presentModeCount > 0, "Found no supported presentation modes!");

    // Query presentation modes
    swapChainSupport.presentModes.resize(presentModeCount);
    VK_VERIFY_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, swapChainSupport.presentModes.data()));

    return swapChainSupport;
}

QueueFamilyIndices HelloTriangle::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices{};

    // Query for queue family count and properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    bool foundGraphics = false;
    bool foundSurface = false;
    VkBool32 presentSupport = false;

    // Iterate over queue familys
    for(const auto& queueFamily : queueFamilies)
    {
        // Query for first graphics capable queue
        if(!foundGraphics && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            foundGraphics = true;
        }

        // Query for first surface presentation queue
        VK_VERIFY_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &presentSupport));
        if(!foundSurface && presentSupport)
        {
            indices.presentFamily = i;
            foundSurface = true;
        }

        // PERF: This will be most likely be the same queue index, it's possible to improve performance here.
        i++;
    }
    ASSERT(foundGraphics , "Found no graphics capable queue family!");
    ASSERT(foundSurface , "Found no surface presentation queue family!");

    return indices;
}

void HelloTriangle::CreateLogicalDevice()
{
    // Create vector with queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    // Add graphics queue
    VkDeviceQueueCreateInfo queueCreateInfo1{};
    queueCreateInfo1.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo1.queueFamilyIndex = _queueFamilyIndices.graphicsFamily;
    queueCreateInfo1.queueCount = 1;
    queueCreateInfo1.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo1);

    // Add surface presentation queue
    VkDeviceQueueCreateInfo queueCreateInfo2{};
    queueCreateInfo2.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo2.queueFamilyIndex = _queueFamilyIndices.presentFamily;
    queueCreateInfo2.queueCount = 1;
    queueCreateInfo2.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo2);

    // Define features you want to use (e.g. geometry shaders)
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Configure logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledLayerCount = 0; // Device specific validation layers get ignored in newer vulkan versions

    // Create logical device
    VK_VERIFY_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));
    LOG_INFO("Created device!");

    // TODO: Reduce to one queue handle

    // Retrieve graphics queue handle
    vkGetDeviceQueue(_device, _queueFamilyIndices.graphicsFamily, 0, &_graphicsQueue);

    // Retrieve surface presentation queue handle
    vkGetDeviceQueue(_device, _queueFamilyIndices.presentFamily, 0, &_presentQueue);
}

void HelloTriangle::CreateSwapChain()
{
    // Swap chain got already queried at this point, so extract all the preferable settings
    _swapChainProperties.extent = ChooseSwapExtent(_swapChainSupport.capabilities);
    _swapChainProperties.surfaceFormat = ChooseSwapSurfaceFormat(_swapChainSupport.formats);
    _swapChainProperties.presentMode = ChooseSwapPresentMode(_swapChainSupport.presentModes);

    // Specify amount of images in swap chain
    uint32_t imageCount = _swapChainSupport.capabilities.minImageCount + 1;

    // Make sure to not exceed bounds (0 := means no limit)
    if(_swapChainSupport.capabilities.maxImageCount > 0 && imageCount > _swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = _swapChainSupport.capabilities.maxImageCount;
    }

    // Create swap chain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = _swapChainProperties.surfaceFormat.format;
    createInfo.imageColorSpace = _swapChainProperties.surfaceFormat.colorSpace;
    createInfo.imageExtent = _swapChainProperties.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Render directly to image

    // Check if graphics queue is different from presentation queue
    if(_queueFamilyIndices.graphicsFamily != _queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Images can be used across multiple queue families
        createInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = {_queueFamilyIndices.graphicsFamily, _queueFamilyIndices.presentFamily};
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Image is owned by one queue family at a time
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = _swapChainSupport.capabilities.currentTransform; // Disable automatic transformation

    // NOTE: This alpha channel config is not yet fully clear to me
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore alpha channel for blending with other os windows ?

    createInfo.presentMode = _swapChainProperties.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE; // NOTE: You will need to revisit this because of window resizing

    VK_VERIFY_RESULT(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain));

    // TODO: Log swap chain parameters (e.g. format, colorSpace ...)
    LOG_INFO("Created swap chain!");

    // Retrieve handles for swap chain images
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());
}
