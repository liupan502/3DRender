#pragma once
#include <rhi/rhi_resource.h>
namespace rhi{
    namespace vulkan {
        class VulkanGraphicsPipeline : public GraphicsPipeline {
        public:
            VulkanGraphicsPipeline(const GraphicsPipelineCreateInfo& info) : GraphicsPipeline(info) {};
        };
    };
};