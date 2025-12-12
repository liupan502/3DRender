#pragma once
#include <core/pipeline_manager.h>
#include <memory>
namespace zr{
    namespace sg{
        class Scene;
    };

    namespace core {
        class FgRenderPass;
        class PipelineManager;
        class CommandBuffer;
        // class CreatePipelineFunc;

        typedef enum AAOption{
            AA_OPTION_NONE,
            AA_OPTION_MSAA,
            AA_OPTION_TAA,
            AA_OPTION_MAX
        } AAOption;

        struct CameraInfo {
            glm::mat4 view_mat;
            glm::mat4 proj_mat;
        };

        class RendererInterface{
        public:
            virtual void prepare_renderpass(sg::Scene* scene, 
                                FgRenderPass* renderpass,
                                std::shared_ptr<Device> device) = 0;
            virtual void render_scene(sg::Scene* scene, 
                        std::shared_ptr<CommandBuffer> cmd_buf) = 0;

            virtual             

            inline  void set_pipeline_mgr(std::shared_ptr<PipelineManager> mgr) {
                _pipeline_mgr = mgr;
            }

            virtual CreatePipelineFunc get_pipeline_creator() = 0;

            inline std::shared_ptr<PipelineManager> get_pipeline_mgr() {
                return _pipeline_mgr;
            }
            
            inline void set_viewport(VkViewport viewport) { _viewport = viewport;};

            inline void set_aa_option(AAOption aa_option) { _aa_option = aa_option;};

            inline AAOption get_aa_option() const { return _aa_option; };

            inline void set_camera_info(CameraInfo* camera_info) { _camera_info = camera_info;};
        protected:
            std::shared_ptr<PipelineManager> _pipeline_mgr;
            VkViewport _viewport;
            AAOption _aa_option{AA_OPTION_NONE};
            CameraInfo* _camera_info{nullptr};
        };
    }
}
