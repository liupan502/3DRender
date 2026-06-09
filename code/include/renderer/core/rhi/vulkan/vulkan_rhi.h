#pragma once
#include "rhi/rhi.h"
#include <render_context.h>
#include <memory>
#include <map>
#include <vector>

namespace rhi { 

    struct RenderPassKey {
        struct AttDesc {
            ColorFormat fmt = ColorFormat::None;
            SampleCount samples = SampleCount::SC_COUNT_1;
            AttachmentLoadOp load_op = AttachmentLoadOp::ALO_LOAD;
            AttachmentStoreOp store_op = AttachmentStoreOp::ASO_STORE;

            bool operator<(const AttDesc& rhs) const {
                if (fmt != rhs.fmt) return fmt < rhs.fmt;
                if (samples != rhs.samples) return samples < rhs.samples;
                if (load_op != rhs.load_op) return load_op < rhs.load_op;
                return store_op < rhs.store_op;
            }
            bool operator==(const AttDesc& rhs) const {
                return fmt == rhs.fmt && samples == rhs.samples &&
                       load_op == rhs.load_op && store_op == rhs.store_op;
            }
        };

        std::vector<AttDesc> color_attachments;
        AttDesc depth_attachment;

        bool operator<(const RenderPassKey& rhs) const {
            if (color_attachments != rhs.color_attachments)
                return color_attachments < rhs.color_attachments;
            return depth_attachment < rhs.depth_attachment;
        }
    };

    class VulkanRHI : public RHI {
    public:
        VulkanRHI() = default;
        ~VulkanRHI() override;
        void init(const void* window) override;
        void destroy() override;

        BufferRef create_buffer(const BufferCreateInfo& info) override;

        void update_buffer(BufferRef buf, void* data, uint32_t len, uint32_t offset) override;

        TextureRef create_texture(const TextureCreateInfo& info) override;

        void update_texture(TextureRef tex, void* data, uint32_t len, 
            uint32_t base_layer, uint32_t mip_level, bool generated_mip_map) override;

        SampleStateRef create_sample_state(const SampleStateCreateInfo& info) override;
    
        ShaderModuleRef create_shader_module(const ShaderModuleCreateInfo& info) override;
        
        GraphicsPipelineRef create_graphics_pipeline(const GraphicsPipelineCreateInfo& info) override;
    
        RenderTargetRef create_render_target(const RenderTargetCreateInfo& info) override;
    
        DescriptorSetLayoutRef create_descriptor_set_layout(const DescriptorSetLayoutCreateInfo& ci) override;

        DescriptorSetRef create_descriptor_set(DescriptorSetLayoutRef layout) override;

        void update_desc_texture(DescriptorSetRef desc, SampleStateRef sampler, TextureRef tex, uint32_t binding_idx) override;

        void begin_render_pass(RenderTargetRef rt, const RenderPassParams& params) override;

        void end_render_pass() override;

        void update_desc_buffer(DescriptorSetRef desc, BufferRef buf, uint32_t binding_idx, uint32_t offset, uint32_t len) override;

        void draw(GraphicsPipelineRef pipeline, const RenderPrimitive& primitive, 
            uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) override;

        inline std::shared_ptr<zr::RenderContext> get_context() const { return _context; }

    private:
        VkRenderPass get_or_create_render_pass(const RenderTargetCreateInfo& rt_ci);

    private:
        std::shared_ptr<zr::RenderContext> _context;
        VkRenderPass _current_render_pass{VK_NULL_HANDLE};
        uint32_t _current_subpass{0};
        std::map<std::pair<vulkan::VulkanGraphicsPipeline*, VkRenderPass>, VkPipeline> _pipeline_cache;
        std::map<RenderPassKey, VkRenderPass> _render_pass_cache;
    };
}