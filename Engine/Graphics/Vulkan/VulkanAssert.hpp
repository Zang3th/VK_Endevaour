#pragma once

#include "Debug/Log.hpp"

#include "vulkan/vulkan.hpp"

namespace Engine
{
    #define VK_VERIFY(func) \
    { \
        vk::Result res = (func); \
        if(res != vk::Result::eSuccess) \
        { \
            LOG_ERROR("######################"); \
            LOG_ERROR("##### VK_VERFIY ######"); \
            LOG_ERROR("######################"); \
            LOG_ERROR("{}\n{:>92}:{}", vk::to_string(res), __FILE__, __LINE__); \
            ASSERT(false, "Caught an error with VK_VERIFY!"); \
        } \
    }
}
