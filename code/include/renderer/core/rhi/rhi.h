#pragma once

#include <rhi/rhi_resource.h>
#include <rhi/rhi_definitions.h>
#include <memory>
namespace rhi
{
    using BufferRef = std::shared_ptr<Buffer>;
    using TextureRef = std::shared_ptr<Texture>;
    using SampleStateRef = std::shared_ptr<SampleState>;
    using ShaderModuleRef = std::shared_ptr<ShaderModule>;
    using GraphicsPipelineRef = std::shared_ptr<GraphicsPipeline>;
    using RenderTargetRef = std::shared_ptr<RenderTarget>;
    using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
    using DescriptorSetLayoutRef = std::shared_ptr<DescriptorSetLayout>;

    class RHI {
    public:
        RHI() = default;
        virtual ~RHI() = default;

        virtual void init(const void* window) = 0;
        virtual void destroy() = 0;


        virtual BufferRef create_buffer(const BufferCreateInfo& info) = 0;

        virtual void update_buffer(BufferRef buf, void* data, uint32_t len, uint32_t offset) = 0;

        virtual TextureRef create_texture(const TextureCreateInfo& info) = 0;

        virtual void update_texture(TextureRef tex, void* data, uint32_t len,
            uint32_t base_layer, uint32_t mip_level, bool generated_mip_map) = 0;

        virtual SampleStateRef create_sample_state(const SampleStateCreateInfo& info) = 0;
        
        virtual ShaderModuleRef create_shader_module(const ShaderModuleCreateInfo& info) = 0;

        virtual GraphicsPipelineRef create_graphics_pipeline(const GraphicsPipelineCreateInfo& info) = 0;
    
        virtual RenderTargetRef create_render_target(const RenderTargetCreateInfo& info) = 0;

        virtual DescriptorSetLayoutRef create_descriptor_set_layout(const DescriptorSetLayoutCreateInfo& ci) = 0;

        virtual DescriptorSetRef create_descriptor_set(DescriptorSetLayoutRef layout) = 0;

        virtual void update_desc_buffer(DescriptorSetRef desc, BufferRef buf, uint32_t binding_idx, uint32_t offset, uint32_t len) = 0;
        
        virtual void update_desc_texture(DescriptorSetRef desc, SampleStateRef sampler, TextureRef tex, uint32_t binding_idx) = 0;

        virtual void begin_render_pass(RenderTargetRef rt, const RenderPassParams& params) = 0;

        virtual void end_render_pass() = 0;

        virtual void draw(GraphicsPipelineRef pipeline, const RenderPrimitive& primitive, 
            uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) = 0;
    };

    extern RHI* rhi_instance;

    void init(const void* window_handle);
}