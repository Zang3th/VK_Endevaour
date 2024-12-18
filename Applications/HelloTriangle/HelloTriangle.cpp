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
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: LOG_VERBOSE("[VkMsg | {}]\n{}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    LOG_INFO   ("[VkMsg | {}]\n{}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: LOG_WARN   ("[VkMsg | {}]\n{}", VkMsgTypeToString(messageType), pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   LOG_ERROR  ("[VkMsg | {}]\n{}", VkMsgTypeToString(messageType), pCallbackData->pMessage); ASSERT(false, "VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT");
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

static std::vector<char> ReadFileAsBytes(const std::string& fileName)
{
    // Start reading file from the end
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    ASSERT(file.is_open(), "Can't open file: {}", fileName);

    // After getting the size, set pointer to begin and read into buffer
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    LOG_INFO("Read in file: {} ({} bytes)", fileName, fileSize);

    return buffer;
}

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto* app = reinterpret_cast<HelloTriangle*>(glfwGetWindowUserPointer(window));

    if(static_cast<uint32_t>(width)  != app->_swapChainProperties.extent.width ||
       static_cast<uint32_t>(height) != app->_swapChainProperties.extent.height)
    {
        LOG_WARN("GLFW: Swapchain may needs recreation (New dimensions: {}x{})", width, height);
        app->framebufferResized = true;
    }
}

// ----- Public -----

void HelloTriangle::Run()
{
    InitWindow();
    InitVulkan();
    InitImGui();
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

    _window = glfwCreateWindow(WIDTH, HEIGHT, "HelloTriangle", nullptr, nullptr);
    ASSERT(_window, "Failed to create GLFW window: {}", glfwGetError(nullptr));
    LOG_INFO("Created GLFW window!");

    // Save application context and set framebuffer resize callback
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
}

// TODO: Replace with macro
static void imgui_check_vk_result(VkResult err)
{
    if (err == 0)
    {
        return;
    }

    LOG_ERROR("[Imgui::Vulkan] VkResult: {}", VkResultToString(err));

    if (err < 0)
    {
        abort();
    }
}

void HelloTriangle::InitImGui()
{
    // Create a very oversized descriptor pool for Imgui
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VK_VERIFY_RESULT(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_imguiPool));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Fetch for current swap chain dimensions
    // RecreateImGuiContext();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(_window, true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = _instance;
    initInfo.PhysicalDevice = _physicalDevice;
    initInfo.Device = _device;
    initInfo.QueueFamily = _queueFamilyIndices.graphicsFamily;
    initInfo.Queue = _graphicsQueue;
    initInfo.PipelineCache = nullptr;
    initInfo.DescriptorPool = _imguiPool;
    initInfo.RenderPass = _renderPass;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = _minImageCount;
    initInfo.ImageCount = _imageCount;
    initInfo.MSAASamples = _msaaSamples;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = imgui_check_vk_result;
    ImGui_ImplVulkan_Init(&initInfo);

    LOG_INFO("Initialized ImGui!");
}

void HelloTriangle::RecreateImGuiContext()
{
    // TODO: Solve dynamically
    // ImGui_ImplVulkan_SetMinImageCount(_minImageCount);
}

void HelloTriangle::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    CreateTextureImage(TEXTURE_PATH);
    CreateTextureImageView();
    CreateTextureSampler();
    LoadModel(MODEL_PATH);
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void HelloTriangle::MainLoop()
{
    while(!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        DrawFrame();
    }

    VK_VERIFY_RESULT(vkDeviceWaitIdle(_device));
}

void HelloTriangle::CleanUp()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(_device, _imguiPool, nullptr);

    CleanupSwapChain();

    vkDestroySampler(_device, _textureSampler, nullptr);
    vkDestroyImageView(_device, _textureImageView, nullptr);

    vkDestroyImage(_device, _textureImage, nullptr);
    vkFreeMemory(_device, _textureImageMemory, nullptr);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

    vkDestroyBuffer(_device, _vertexBuffer, nullptr);
    vkFreeMemory(_device, _vertexBufferMemory, nullptr);

    vkDestroyBuffer(_device, _indexBuffer, nullptr);
    vkFreeMemory(_device, _indexBufferMemory, nullptr);

    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);

    vkDestroyRenderPass(_device, _renderPass, nullptr);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(_device, _commandPool, nullptr);

    vkDestroyDevice(_device, nullptr);

    if(enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }

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
            _msaaSamples = GetMSAASampleCount();
            break;
        }
    }
    // TODO: Centralize logging of device specifics
    LOG_INFO("Device supports MSAA with {} samples!", static_cast<uint32_t>(_msaaSamples));

    ASSERT(_physicalDevice, "Failed to find suitable device!");

    // TODO: Move this also into the IsDeviceSuitable()-Function
    _queueFamilyIndices = FindQueueFamilies(_physicalDevice);
    LOG_INFO("Found graphics capable queue family (index {})", _queueFamilyIndices.graphicsFamily);
    LOG_INFO("Found surface presentation queue family (index {})", _queueFamilyIndices.presentFamily);
}

bool HelloTriangle::IsDeviceSuitable(VkPhysicalDevice device)
{
    // Query device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Check for requirements (discrete GPU and support for extensions)
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && CheckDeviceExtensionSupport(device))
    {
        LOG_INFO("Found discrete GPU with support for all requested extensions!");

        // Query swap chain support
        _swapChainSupport = QuerySwapChainSupport(device);

        if(!_swapChainSupport.formats.empty() && !_swapChainSupport.presentModes.empty())
        {
            LOG_INFO("Device has adequate swap chain support!");

            // Query device features
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            if(deviceFeatures.samplerAnisotropy)
            {
                LOG_INFO("Device supports anisotropic filtering!");

                LOG_INFO("GPU: {} ({}), Driver: {}", deviceProperties.deviceName, VkVendorIDToString(deviceProperties.vendorID), VkDriverVersionToString(deviceProperties.driverVersion));

                return true;
            }
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
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

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
    // Query for swap chain properties
    _swapChainSupport = QuerySwapChainSupport(_physicalDevice);
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

    LOG_INFO("Created swap chain: {}x{}, Format: {}",
             _swapChainProperties.extent.width, _swapChainProperties.extent.height,
             (uint32_t)_swapChainProperties.surfaceFormat.format);

    // Retrieve handles for swap chain images
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());
}

VkImageView HelloTriangle::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    // Define image view
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    // Create image view
    VkImageView imageView;
    VK_VERIFY_RESULT(vkCreateImageView(_device, &imageViewInfo, nullptr, &imageView));

    return imageView;
}

void HelloTriangle::CreateImageViews()
{
    _swapChainImageViews.resize(_swapChainImages.size());

    // Iterate over all swap chain images
    for(size_t i = 0; i < _swapChainImages.size(); i++)
    {
        // Create image views
        _swapChainImageViews[i] = CreateImageView(_swapChainImages[i], _swapChainProperties.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    LOG_INFO("Created image views!");
}

void HelloTriangle::CreateRenderPass()
{
    // Define color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _swapChainProperties.surfaceFormat.format;
    colorAttachment.samples = _msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Define color attachment reference
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Define depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT; // TODO: Save in class variable
    depthAttachment.samples = _msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Define depth attachment reference
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Define color attachment resolve
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = _swapChainProperties.surfaceFormat.format;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Define color attachment resolve reference
    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Define subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1; // This directly corresponds to *out vec4 outColor* in the fragment shader!
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    // Define subpass dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Create render pass
    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_VERIFY_RESULT(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass));
    LOG_INFO("Created render pass!");
}

VkShaderModule HelloTriangle::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // Bytecode pointer is uint32_t

    VkShaderModule shaderModule;
    VK_VERIFY_RESULT(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));
    LOG_INFO("Created shader module!");

    return shaderModule;
}

void HelloTriangle::CreateGraphicsPipeline()
{
    // Read in compiled shader code
    auto vertShaderCode = ReadFileAsBytes("Shaders/Vert.spv");
    auto fragShaderCode = ReadFileAsBytes("Shaders/Frag.spv");

    // Create shader modules
    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    // Create vertex shader stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr; // It's possible to set constants here

    // Create vertex shader stage
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr; // It's possible to set constants here

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Specify dynamic states (can be changed without recreating the whole pipeline)
    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    // Define vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Define input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // Define viewport and scissor count
    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount = 1;

    // Define rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;

    // Disable multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.sampleShadingEnable = VK_TRUE;
    multisamplingInfo.minSampleShading = 0.2f;
    multisamplingInfo.rasterizationSamples = _msaaSamples;

    // Disable blending (new fragment colors will just get passed through to the framebuffer)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE; // Set true for bitwise combination
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;

    // TODO: Implement alpha blending (https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions#page_Color-blending)

    // Define front and back stencil op states even though we don't use them at the moment
    VkStencilOpState frontStencilState{};
    frontStencilState.failOp = VK_STENCIL_OP_KEEP;
    frontStencilState.passOp = VK_STENCIL_OP_REPLACE;
    frontStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
    frontStencilState.compareOp = VK_COMPARE_OP_ALWAYS;

    VkStencilOpState backStencilState{};
    backStencilState.failOp = VK_STENCIL_OP_KEEP;
    backStencilState.passOp = VK_STENCIL_OP_REPLACE;
    backStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
    backStencilState.compareOp = VK_COMPARE_OP_ALWAYS;

    // Define depth stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.flags = 0;
    depthStencilInfo.front = frontStencilState;
    depthStencilInfo.back = backStencilState;

    // Define pipeline layout (for uniforms)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    VK_VERIFY_RESULT(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout));

    // Finally create the pipeline itself
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;

    VK_VERIFY_RESULT(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));
    LOG_INFO("Created graphics pipeline!");

    // Destroy shader modules
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
}

void HelloTriangle::CreateFramebuffers()
{
    _swapChainFramebuffers.resize(_swapChainImageViews.size());

    // Iterate over all image views and create framebuffers from them
    for(size_t i = 0; i < _swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 3> attachments = {_colorImageView, _depthImageView, _swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapChainProperties.extent.width;
        framebufferInfo.height = _swapChainProperties.extent.height;
        framebufferInfo.layers = 1;

        VK_VERIFY_RESULT(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]));
    }
}

void HelloTriangle::CreateCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo{};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Rerecord cmd buffer every frame
    cmdPoolInfo.queueFamilyIndex = _queueFamilyIndices.graphicsFamily;

    VK_VERIFY_RESULT(vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_commandPool));
    LOG_INFO("Created command pool!");
}

void HelloTriangle::CreateCommandBuffers()
{
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo cmdBufferInfo{};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferInfo.commandPool = _commandPool;
    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    VK_VERIFY_RESULT(vkAllocateCommandBuffers(_device, &cmdBufferInfo, _commandBuffers.data()));
    LOG_INFO("Created command buffers!");
}

void HelloTriangle::RecordCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // Begin command recording
    VkCommandBufferBeginInfo cmdBufferBeginInfo{};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_VERIFY_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo));

    // Begin render pass
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _swapChainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = _swapChainProperties.extent;

    // Define clear color
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    // Bind vertex buffer
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_vertexBuffer, offsets);

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // Bind descriptor sets
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_currentFrame], 0, nullptr);

    // Define viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChainProperties.extent.width);
    viewport.height = static_cast<float>(_swapChainProperties.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChainProperties.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Draw indexed
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);

    // Draw imgui stuff as part of the main renderpass
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    // End render pass and end command buffer recording
    vkCmdEndRenderPass(commandBuffer);
    VK_VERIFY_RESULT(vkEndCommandBuffer(commandBuffer));
}

void HelloTriangle::CreateSyncObjects()
{
    // Reserve space
    _imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    // Define semaphores
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Define fence
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Signaled state to avoid endless waiting for first frame

    // Create sync objects
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VK_VERIFY_RESULT(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]));
        VK_VERIFY_RESULT(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]));
        VK_VERIFY_RESULT(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]));
    }

    LOG_INFO("Created sync objects!");
}

void HelloTriangle::DrawFrame()
{
    ImGui::Render();

    // Wait for previous frame to finish
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    // Get image from the swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        LOG_WARN("vkAcquireNextImageKHR recreated swap chain!");
        return; // Try again next frame
    }
    ASSERT(result == VK_SUCCESS, "Failed to acquire swap chain image!");

    // Update uniform buffer
    UpdateUniformBuffer(_currentFrame);

    // Reset fence only if we are submitting work
    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);

    // Record command buffer
    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);
    RecordCommands(_commandBuffers[_currentFrame], imageIndex);

    // Submit command buffer to graphics queue
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &_imageAvailableSemaphores[_currentFrame]; // On which semaphore to wait
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_renderFinishedSemaphores[_currentFrame]; // Which semaphore to signal after

    VK_VERIFY_RESULT(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]));

    // Presentation
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &_renderFinishedSemaphores[_currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapChain;
    presentInfo.pImageIndices = &imageIndex;

    // Check if swap chain needs recreation
    result = vkQueuePresentKHR(_presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        LOG_WARN("vkQueuePresentKHR recreated swap chain!");
        framebufferResized = false;
        RecreateSwapChain();
    }
    else
    {
        ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
    }

    // Advance to next frame
    _currentFrame = (_currentFrame + 1) & MAX_FRAMES_IN_FLIGHT;
}

// TODO: Fix
void HelloTriangle::RenderImGuiFrame()
{
    // // Reset command pool
    // err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
    // check_vk_result(err);
    // VkCommandBufferBeginInfo info = {};
    // info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    // check_vk_result(err);
    //
    // // Begin render pass
    // VkRenderPassBeginInfo info = {};
    // info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // info.renderPass = wd->RenderPass;
    // info.framebuffer = fd->Framebuffer;
    // info.renderArea.extent.width = wd->Width;
    // info.renderArea.extent.height = wd->Height;
    // info.clearValueCount = 1;
    // info.pClearValues = &wd->ClearValue;
    // vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    //
    // // Record dear imgui primitives into command buffer
    // ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);
    //
    // // Submit command buffer
    // vkCmdEndRenderPass(fd->CommandBuffer);
    //
    // VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // VkSubmitInfo info = {};
    // info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // info.waitSemaphoreCount = 1;
    // info.pWaitSemaphores = &image_acquired_semaphore;
    // info.pWaitDstStageMask = &wait_stage;
    // info.commandBufferCount = 1;
    // info.pCommandBuffers = &fd->CommandBuffer;
    // info.signalSemaphoreCount = 1;
    // info.pSignalSemaphores = &render_complete_semaphore;
    //
    // // End render pass
    // err = vkEndCommandBuffer(fd->CommandBuffer);
    // check_vk_result(err);
    // err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
    // check_vk_result(err);
    //
    // // Present
    // if (g_SwapChainRebuild)
    //     return;
    //
    // VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    // VkPresentInfoKHR info = {};
    // info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // info.waitSemaphoreCount = 1;
    // info.pWaitSemaphores = &render_complete_semaphore;
    // info.swapchainCount = 1;
    // info.pSwapchains = &wd->Swapchain;
    // info.pImageIndices = &wd->FrameIndex;
    //
    // VkResult err = vkQueuePresentKHR(g_Queue, &info);
    // if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    // {
    //     g_SwapChainRebuild = true;
    //     return;
    // }
    // check_vk_result(err);
    // wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

void HelloTriangle::CleanupSwapChain()
{
    vkDestroyImageView(_device, _colorImageView, nullptr);
    vkDestroyImage(_device, _colorImage, nullptr);
    vkFreeMemory(_device, _colorImageMemory, nullptr);

    vkDestroyImageView(_device, _depthImageView, nullptr);
    vkDestroyImage(_device, _depthImage, nullptr);
    vkFreeMemory(_device, _depthImageMemory, nullptr);

    for(auto* framebuffer : _swapChainFramebuffers)
    {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }

    for(auto* imageView : _swapChainImageViews)
    {
        vkDestroyImageView(_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

void HelloTriangle::RecreateSwapChain()
{
    vkDeviceWaitIdle(_device); // TODO: Check if this is really the best synchronisation method here

    CleanupSwapChain();
    CreateSwapChain();
    CreateImageViews();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    RecreateImGuiContext();
}

uint32_t HelloTriangle::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // Query available memory
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    // Find suitable memory
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    ASSERT(false, "Failed to find suitable memory type!");
    return -1;
}

void HelloTriangle::CreateVertexBuffer()
{
    // Get buffer size
    VkDeviceSize bufferSize = sizeof(Vertex) * _vertices.size();

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    // Copy/map vertex data to buffer
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_device, stagingBufferMemory);

    // Create device local buffer
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_vertexBuffer, &_vertexBufferMemory);

    // Copy vertex data to device local buffer
    CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    // Destroy staging buffer
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void HelloTriangle::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
    // Define vertex buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Create vertex buffer
    VK_VERIFY_RESULT(vkCreateBuffer(_device, &bufferInfo, nullptr, buffer));

    // Check memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device, *buffer, &memRequirements);

    // Define memory allocation
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    // Allocate memory
    VK_VERIFY_RESULT(vkAllocateMemory(_device, &allocInfo, nullptr, bufferMemory));

    // Bind memory to buffer
    vkBindBufferMemory(_device, *buffer, *bufferMemory, 0);
}

VkCommandBuffer HelloTriangle::BeginSingleTimeCommands()
{
    // Define memory transfer operation via command buffers
    // TODO: Define separate command pool for short-lived buffers
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    // Create command buffer
    VkCommandBuffer commandBuffer;
    VK_VERIFY_RESULT(vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer));

    // Begin recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_VERIFY_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    return commandBuffer;
}

void HelloTriangle::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    // End recording
    VK_VERIFY_RESULT(vkEndCommandBuffer(commandBuffer));

    // Immediately submit recorded commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_VERIFY_RESULT(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_VERIFY_RESULT(vkQueueWaitIdle(_graphicsQueue));

    // Clean up command buffer
    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void HelloTriangle::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

// TODO: Check if you want to use a single buffer for vertex and index data (more cache friendly)
void HelloTriangle::CreateIndexBuffer()
{
    // Get buffer size
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    // Copy/map index data to buffer
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_device, stagingBufferMemory);

    // Create device local buffer
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_indexBuffer, &_indexBufferMemory);

    // Copy index data to device local buffer
    CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);

    // Destroy staging buffer
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void HelloTriangle::CreateDescriptorSetLayout()
{
    // Define ubo layout
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Define combined image sampler layout
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    // Define descriptor set
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
    descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorLayoutInfo.pBindings = bindings.data();

    // Create descriptor set
    VK_VERIFY_RESULT(vkCreateDescriptorSetLayout(_device, &descriptorLayoutInfo, nullptr, &_descriptorSetLayout));

    LOG_INFO("Created descriptor set layout!");
}

void HelloTriangle::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // Resize vectors
    _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_uniformBuffers[i], &_uniformBuffersMemory[i]);
        vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
}

void HelloTriangle::UpdateUniformBuffer(uint32_t frame)
{
    // TODO: Create glfw based frametime
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // Create ubo
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(_swapChainProperties.extent.width) / static_cast<float>(_swapChainProperties.extent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; // Invert sign on the scaling factor of the Y axis (legacy of OpenGL)

    // Copy data to mapped uniform buffer
    memcpy(_uniformBuffersMapped[frame], &ubo, sizeof(ubo));
}

void HelloTriangle::CreateDescriptorPool()
{
    // Define pool size
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSize{};
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // Define descriptor pool layout
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
    descriptorPoolInfo.pPoolSizes = descriptorPoolSize.data();
    descriptorPoolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // Create descriptor pool
    VK_VERIFY_RESULT(vkCreateDescriptorPool(_device, &descriptorPoolInfo, nullptr, &_descriptorPool));
}

void HelloTriangle::CreateDescriptorSets()
{
    // Define descriptor set allocation
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorAllocInfo{};
    descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocInfo.descriptorPool = _descriptorPool;
    descriptorAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorAllocInfo.pSetLayouts = layouts.data();

    // Create descriptor set
    _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VK_VERIFY_RESULT(vkAllocateDescriptorSets(_device, &descriptorAllocInfo, _descriptorSets.data()));

    // Populate every descriptor
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = _uniformBuffers[i];
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo descriptorImageInfo{};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView = _textureImageView;
        descriptorImageInfo.sampler = _textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &descriptorImageInfo;

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

// TODO: Create structure
void HelloTriangle::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory)
{
    // Define texture image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;

    // Create texture image
    VK_VERIFY_RESULT(vkCreateImage(_device, &imageInfo, nullptr, image));
    LOG_INFO("Created image: {}x{}, Format: {}", width, height, (uint32_t)format);

    // Define memory allocation
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate memory
    VK_VERIFY_RESULT(vkAllocateMemory(_device, &allocInfo, nullptr, imageMemory));

    vkBindImageMemory(_device, *image, *imageMemory, 0);
}

void HelloTriangle::CreateTextureImage(const std::string& filepath)
{
    // Load image
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    ASSERT(pixels, "Failed to load image: {}", filepath);
    LOG_INFO("Loaded image: {} ({}x{}, {} channels)", filepath, texWidth, texHeight, texChannels);

    // Calculate mip levels
    _mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    // Copy image data to buffer
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(_device, stagingBufferMemory);

    stbi_image_free(pixels);

    // Create image
    CreateImage(texWidth, texHeight, _mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_textureImage, &_textureImageMemory);

    // Transition image layout for copying
    TransitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _mipLevels);

    // Copy staging buffer to texture image
    CopyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    // Transitioned image layout for sampling in the shader while generating mipmaps
    GenerateMipmaps(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, _mipLevels);
}

// TODO: Create a SetupCommandBuffer funtion that all helper function record commands into, flush once
void HelloTriangle::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    // Define image memory barrier
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = 0;
    VkPipelineStageFlags dstStage = 0;

    // Set access masks and pipeline stages based on layout
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        ASSERT(false, "Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

void HelloTriangle::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    // Define buffer image copy
    VkBufferImageCopy imageCopy{};
    imageCopy.bufferOffset = 0;
    imageCopy.bufferRowLength = 0;
    imageCopy.bufferImageHeight = 0;
    imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopy.imageSubresource.mipLevel = 0;
    imageCopy.imageSubresource.baseArrayLayer = 0;
    imageCopy.imageSubresource.layerCount = 1;
    imageCopy.imageOffset = {0, 0, 0};
    imageCopy.imageExtent = {width, height, 1};

    // Copy buffer to image
    // NOTE: The 4th parameter requires the image to already be in the correct format!
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

    EndSingleTimeCommands(commandBuffer);
}

void HelloTriangle::CreateTextureImageView()
{
    _textureImageView = CreateImageView(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);
}

void HelloTriangle::CreateTextureSampler()
{
    // Define texture sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    // Query phyiscal device for maximum possible amount of texels to sample
    // TODO: Move into class variable that's only getting queried once
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(_mipLevels);

    // Create texture sampler
    VK_VERIFY_RESULT(vkCreateSampler(_device, &samplerInfo, nullptr, &_textureSampler));

    LOG_INFO("Created texture sampler!");
}

void HelloTriangle::CreateDepthResources()
{
    // TODO: Optional query for more optimal format possible here
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    uint32_t width = _swapChainProperties.extent.width;
    uint32_t height = _swapChainProperties.extent.height;

    // Create image and image view
    CreateImage(width, height, 1, _msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_depthImage, &_depthImageMemory);
    _depthImageView = CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void HelloTriangle::LoadModel(const std::string& filepath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, error;

    // Load file (already triangulated)
    ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filepath.c_str()), "Failed to load model: {} | {}", warn, error);
    LOG_INFO("Loaded obj model: {} (Vertices: {})", filepath, attrib.vertices.size());

    // Cache and reuse vertices (needs a hashing function and overload comparison operator. TODO: Simplify)
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    // Combine all faces into a single model by iterating over all shapes
    for(const auto& shape : shapes)
    {
        for(const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // Flip v axis
            };

            if(!uniqueVertices.contains(vertex))
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                _vertices.push_back(vertex);
            }

            _indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void HelloTriangle::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties{};
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, imageFormat, &formatProperties);
    ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "Texture image does not support linear blitting!");

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    // Define general memory barrier
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    // Record commands for creating all mip levels
    for(uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        // Define regions that will we used in blit operation
        VkImageBlit blit{};

        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1].x = int32_t(mipWidth >> (i - 1));
        blit.srcOffsets[1].y = int32_t(mipWidth >> (i - 1));
        blit.srcOffsets[1].z = 1;
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1].x = int32_t(mipWidth >> i);
        blit.dstOffsets[1].y = int32_t(mipWidth >> i);
        blit.dstOffsets[1].z = 1;
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        // Record blit command
        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(commandBuffer);

    LOG_INFO("Generated mipmaps: {} levels", mipLevels);
}

VkSampleCountFlagBits HelloTriangle::GetMSAASampleCount()
{
    // TODO: Move out like always
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(_physicalDevice, &deviceProperties);

    VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts & deviceProperties.limits.framebufferDepthSampleCounts;

    // Return highest supported sample count
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT)  { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT)  { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT)  { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void HelloTriangle::CreateColorResources()
{
    VkFormat colorFormat = _swapChainProperties.surfaceFormat.format;
    uint32_t width = _swapChainProperties.extent.width;
    uint32_t height = _swapChainProperties.extent.height;

    CreateImage(width, height, 1, _msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_colorImage, &_colorImageMemory);
    _colorImageView = CreateImageView(_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}
