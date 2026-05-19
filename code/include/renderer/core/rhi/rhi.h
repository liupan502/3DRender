#pragma once

#include <rhi/rhi_resource.h>
#include <memory>
namespace rhi
{
    using BufferRef = std::shared_ptr<Buffer>;
    using TextureRef = std::shared_ptr<Texture>;
    using SampleStateRef = std::shared_ptr<SampleState>;
    using ShaderModuleRef = std::shared_ptr<ShaderModule>;
    using GraphicsPipelineRef = std::shared_ptr<GraphicsPipeline>;
    using RenderTargetRef = std::shared_ptr<RenderTarget>;

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
    
        virtual RenderTargetRef create_render_target(const RenderTargetCreateInfo& info) {
            return std::make_shared<RenderTarget>(info);
        };
    };

    extern RHI* rhi_instance;

    void init(const void* window_handle);
}