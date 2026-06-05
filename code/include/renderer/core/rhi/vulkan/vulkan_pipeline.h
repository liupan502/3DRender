#pragma once
#include <rhi/rhi_resource.h>
#include <vulkan/vulkan.h>
namespace rhi{
    namespace vulkan {
        class VulkanGraphicsPipeline : public GraphicsPipeline {
        public:
            VulkanGraphicsPipeline(const GraphicsPipelineCreateInfo& info) : GraphicsPipeline(info) {};

            inline VkPipeline get() const { return _vk_pipeline; }

        private:
            VkPipeline _vk_pipeline{VK_NULL_HANDLE};
        };
    };
};