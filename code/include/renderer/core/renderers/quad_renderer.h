#pragma once
#include <core/renderers/renderer_interface.h>
#include <core/descriptor.h>
namespace zr {
    namespace sg {
        class Texture;
    };
    namespace core {
        class Buffer;
        class QuadRenderer : public RendererInterface {
        public:
virtual void prepare_renderpass(sg::Scene* scene, FgRenderPass* renderpass, const PassResources& res) override;
            virtual void render_scene(sg::Scene* scene, const PassResources& res) override;
            virtual CreatePipelineFunc get_pipeline_creator() override;

            
        protected:
            virtual void prepare_desc(FgRenderPass* renderpass, const PassResources& res);

            void reset_viewport(FgRenderPass* renderpass);
        protected:
            rhi::BufferRef _quad_rhi_buf;
            rhi::BufferRef _quad_rhi_idx_buf;
            std::shared_ptr<DescriptorSet> _desc_set = nullptr;
            std::shared_ptr<Pipeline> _pipeline = nullptr;
            std::shared_ptr<sg::Texture> _tex = nullptr;
            
        };

        class QuadPipeline : public Pipeline {
            public:
            QuadPipeline(std::shared_ptr<FgRenderPass> render_pass, uint32_t subpass_idx, 
                        PipelineFeature feature, 
                        const std::map<std::string, std::string>& shader_path_map, 
                        const std::vector<rhi::DescriptorBindingInfo>& binding_infos);
            virtual void create_color_blend_state() override;

            virtual void create_vtx_input_state() override;

            virtual void create_depth_stencil_state() override;

            virtual void create_shader_stage() override;

            virtual void create_dynamic_states() override;

            protected:
            std::map<std::string, std::string> _shader_path_map;
        };

        /*class QuadDescriptorLayout : public DescriptorLayout {

        };*/
    };
}
