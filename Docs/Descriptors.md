# Descriptors

Renderer:
  - ~Holds VulkanGlobalUniforms and GlobalUniformData~
  - ~Updates global uniform data per frame~
    - ~Later with correct camera data~
  - ~When creating a pipeline: passes the global descriptor set layout to the pipeline~
  - ~When rendering: binds the global descriptor set for the current frame~

Pipeline:
  - ~Receives descriptor set layout~
  - ~Creates vk::PipelineLayout from descriptor set layout~

VulkanGlobalUniforms:
  - ~Owns VulkanDescriptorSetLayout~
  - ~Owns VulkanDescriptorPool~
  - ~Owns uniform buffers and allocations per frame-in-flight~
  - ~Owns descriptor sets per frame-in-flight~
  - ~Updates GlobalUniformData~
  - ~Exposes descriptor set layout for pipeline creation~
  - ~Exposes descriptor set for rendering~
