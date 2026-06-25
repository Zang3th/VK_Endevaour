#pragma once

#include "Graphics/Vulkan/VulkanAllocator.hpp"
#include "Graphics/Vulkan/VulkanContext.hpp"
#include "Graphics/Vulkan/VulkanDescriptorPool.hpp"
#include "Graphics/Vulkan/VulkanDescriptorSetLayout.hpp"

#include "Vendor/glm/mat4x4.hpp"

namespace Engine::Graphics
{
    struct GlobalUniformData
    {
        alignas(16) glm::mat4 Model;
        alignas(16) glm::mat4 View;
        alignas(16) glm::mat4 Projection;
    };

    class VulkanGlobalUniforms
    {
    public:
        // Owns the per-frame global uniform buffers and the descriptor infrastructure
        VulkanGlobalUniforms(VulkanContext* context);

        // Releases all per-frame uniform buffers; descriptor layout/pool are released by their RAII wrappers
        ~VulkanGlobalUniforms();

        VulkanGlobalUniforms(const VulkanGlobalUniforms&)            = delete;
        VulkanGlobalUniforms& operator=(const VulkanGlobalUniforms&) = delete;

        // Copies the current global uniform data into the uniform buffer belonging to the active frame-in-flight
        void Update(u32 frameIndex, const GlobalUniformData* data);

        [[nodiscard]] const VulkanDescriptorSetLayout* GetLayout() const { return m_DescriptorLayout.get(); };
        [[nodiscard]] const vk::DescriptorSet*         GetDescriptorSet(u32 frameIndex) const
        {
            return &m_DescriptorSets.at(frameIndex);
        }

    private:
        // Creates the descriptor pool, which reserves descriptor-set storage for all per-frame global uniform sets
        void CreatePool();

        // Creates the descriptor set layout, which declares a vertex-stage uniform buffer
        void CreateLayout();

        // Allocates one CPU-updated uniform buffer per frame-in-flight
        void AllocateBuffers();

        // Allocates one descriptor set per frame-in-flight from the pool using the global uniform layout
        void AllocateDescriptorSets();

        // Writes each descriptor set so it references the matching frame's uniform buffer
        void WriteDescriptorSets();

        VulkanContext* m_Context = nullptr;

        Scope<VulkanDescriptorPool>      m_DescriptorPool;
        Scope<VulkanDescriptorSetLayout> m_DescriptorLayout;

        [[maybe_unused]] std::array<BufferAllocation, FRAMES_IN_FLIGHT>  m_BufferAllocs;
        [[maybe_unused]] std::array<vk::DescriptorSet, FRAMES_IN_FLIGHT> m_DescriptorSets;
    };
}
