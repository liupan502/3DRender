#pragma once
#include <rhi/rhi_resource.h>
#include <vulkan/vulkan.h>
#include <memory>

namespace zr {
    class RenderContext;
}

namespace rhi{
    namespace vulkan {
        class VulkanGraphicsPipeline : public GraphicsPipeline {
        public:
            VulkanGraphicsPipeline(std::shared_ptr<zr::RenderContext> context,
                                   const GraphicsPipelineCreateInfo& info);
            ~VulkanGraphicsPipeline();

            VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
            VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

            inline VkPipeline get() const { return _vk_pipeline; }
            inline VkPipelineLayout get_layout() const { return _vk_pipeline_layout; }

            VkPipeline get_or_create(VkRenderPass render_pass, uint32_t subpass);

        private:
            std::shared_ptr<zr::RenderContext> _context;
            VkPipeline _vk_pipeline{VK_NULL_HANDLE};
            VkPipelineLayout _vk_pipeline_layout{VK_NULL_HANDLE};
            VkRenderPass _cached_render_pass{VK_NULL_HANDLE};
            uint32_t _cached_subpass{0};
        };
    };
};