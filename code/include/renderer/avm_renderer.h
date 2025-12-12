#pragma once
#include <vector>
#include <multipass_renderer.h>
#include <scenegraph/components/texture.h>
#include <core/renderers/avm_video_renderer.h>

namespace zr{

    class AvmRenderer : public MultiPassRenderer{
        public:
        AvmRenderer() {
            // _texs = std::vector<std::shared_ptr<sg::SingleLayerTexture>>(11, nullptr);
        }

        void set_avm_mode(core::AvmMode mode); 

        void set_img_size(const glm::vec2& size); 

        void set_img_offset(const glm::vec2& offset);

        void set_focal(float focal);

        void update_texture(core::AvmMode mode, const unsigned char* data,
                    uint32_t data_len, uint8_t com);

        virtual void render_scene(std::shared_ptr<sg::Scene> scene) override;

        inline void set_surround_camera_intri_arr(const std::vector<core::SurroundCameraIntrinsics> arr) {
            _video_renderer->set_surround_camera_intri_arr(arr);
        }

        protected:
        virtual bool init_internel(VkSampleCountFlagBits sample_count) override;
        void add_main_pass();
        void upload_texs(std::shared_ptr<core::Device> device);
        private:
        std::shared_ptr<core::AvmVideoRenderer> _video_renderer;
        core::AvmMode _mode;
        glm::vec2 _img_size;
        std::vector<std::shared_ptr<sg::SingleLayerTexture>> _texs;
    };
}
