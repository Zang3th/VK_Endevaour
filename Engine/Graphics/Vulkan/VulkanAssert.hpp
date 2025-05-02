#pragma once

#include "Debug/Log.hpp"

#include "vulkan/vulkan.hpp"

namespace Engine
{
    #define VK_VERIFY(func) \
    { \
        vk::Result VK_VerifyResult = (func); \
        if(VK_VerifyResult != vk::Result::eSuccess) \
        { \
            LOG_ERROR("######################"); \
            LOG_ERROR("##### VK_VERFIY ######"); \
            LOG_ERROR("######################"); \
            LOG_ERROR("{}\n{:>92}:{}", vk::to_string(VK_VerifyResult), __FILE__, __LINE__); \
            ASSERT(false, "Caught an error with VK_VERIFY!"); \
        } \
    }
}
