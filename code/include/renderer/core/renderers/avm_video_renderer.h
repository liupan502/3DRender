#pragma once
#include <core/renderers/quad_renderer.h>
namespace zr {
    namespace core{
        enum AvmMode {
            AVM_MODE_NONE,
            AVM_MODE_SOURROUND_LEFT,
            AVM_MODE_SOURROUND_RIGHT,
            AVM_MODE_SOURROUND_REAR,
            AVM_MODE_SOURROUND_FRONT,
            AVM_MODE_SIDE_RIGHT_REAR,
            AVM_MODE_SIDE_RIGHT_FRONT,
            AVM_MODE_SIDE_LEFT_REAR,
            AVM_MODE_SIDE_LEFT_FRONT,
            AVM_MODE_WIDE_REAR,
            AVM_MODE_WIDE_FRONT,
            AVM_MODE_LONG,
            AVM_MODE_IPM,
            AVM_MODE_BOWL,
            AVM_MODE_MAX
        };

        struct AvmConfigInfo{
            glm::vec4 frame_info;
            glm::vec4 sampler_info;
        };

        struct SurroundCameraIntrinsics {
            glm::vec4 frame_info; // frame size & distortion center
            glm::vec4 affine;
            glm::mat4 world2cam;
        };

        class AvmVideoRenderer : public QuadRenderer {
            public:
            virtual CreatePipelineFunc get_pipeline_creator() override;

            void set_mode(AvmMode mode);  

            inline void set_surround_camera_intri_arr(const std::vector<SurroundCameraIntrinsics>& arr) {
                _surround_camera_intri_arr = arr;
            }

            inline void set_img_offset(const glm::vec2& offset) {
                _config_info.frame_info.z = offset.x;
                _config_info.frame_info.w = offset.y;
            }

            inline void set_focal(float focal) {
                _config_info.sampler_info.z = focal;
            }

            void set_surround_tex(std::shared_ptr<sg::SingleLayerTexture> tex) { _surround_tex = tex; };

            protected:
            virtual void prepare_desc(FgRenderPass* render_pass, std::shared_ptr<Device> device) override;
            private:
            AvmMode _mode; 
            AvmConfigInfo _config_info;
            std::shared_ptr<sg::SingleLayerTexture> _surround_tex;   
            std::shared_ptr<core::UniformBuffer> _config_uniform_buf;
            std::shared_ptr<core::UniformBuffer> _surround_camera_intri_buf;  
            std::vector<SurroundCameraIntrinsics> _surround_camera_intri_arr;             
        };
    };
};
