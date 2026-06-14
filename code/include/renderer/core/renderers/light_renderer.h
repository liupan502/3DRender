#pragma once
#include <core/renderers/renderer_interface.h>
#include <scenegraph/components/camera.h>

namespace zr{

namespace sg{
    class Camera;
    class Node;
};

namespace core {
    
    struct VelocityUniformBufferObject {
        glm::mat4 current_world_view_proj_mat;
        glm::mat4 pre_world_view_proj_mat;
    };

    class LightRenderer : public RendererInterface {
        public:
        LightRenderer();
        virtual void prepare_renderpass(sg::Scene* scene, FgRenderPass* renderpass, const PassResources& res) override;
        virtual void render_scene(sg::Scene* scene, const PassResources& res) override;
        virtual CreatePipelineFunc get_pipeline_creator() override;

        protected:
        UniformBufferObject create_uniform_buffer_obj(std::shared_ptr<sg::Node> node,
                        std::shared_ptr<sg::Camera> camera);

        std::shared_ptr<Pipeline> get_pipeline(std::shared_ptr<sg::Node> node, const LightInfo& light_info);                

        private:
        std::shared_ptr<UniformBuffer> _uniform_buf;

        std::shared_ptr<UniformBuffer> _velocity_uniform_buf;

        glm::mat4 _pre_view_mat;
        glm::mat4 _pre_proj_mat;

        bool _is_first_frame = true;

        void try_init_uniform_buffers();

        void try_update_light_data(std::shared_ptr<sg::Node> node, 
                                    sg::Scene* scene, std::shared_ptr<core::DescriptorSet> desc_set);                            
    };
};
};
