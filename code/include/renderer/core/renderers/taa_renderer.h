#pragma once

#include <core/renderers/quad_renderer.h>
#include <glm/glm.hpp> 

namespace zr{
    namespace core{

        struct TaaInfo {
            glm::mat4 reproject;

            // weights 拥有9个值， 此处使用36个是为了兼容spir-v的内存对齐方式，
            // 详见update_weights中的赋值位置
            float filter_weights[36];
            float alpha;
        };

        class ImageView;
        
        class TaaRenderer : public QuadRenderer {
            public:
                static glm::vec2 halton(uint8_t idx);

            public:
                TaaRenderer();
                virtual CreatePipelineFunc get_pipeline_creator() override;

                inline void set_frame_size(const glm::vec2& frame_size) { _frame_size = frame_size; };

                inline void set_jitter(const glm::vec2& jitter) { _jitter = jitter; };
                inline const glm::vec2& get_jitter() const { return _jitter; }; 

                // inline void set_reproject_mat(const glm::mat4& reproject) { _taa_info.reproject = reproject; };
                // inline glm::mat4 get_reproject_mat() const { return _taa_info.reproject; };

                inline void set_alpha(float alpha) { _taa_info.alpha = alpha;};
                inline float get_alpha() const { return _taa_info.alpha; };

                inline void set_filter_width(float filter_width) { _filter_width = filter_width ;};
                inline float get_filter_width() const { return _filter_width; };

                virtual void render_scene(sg::Scene* scene, const PassResources& res) override;

            protected:
            virtual void prepare_desc(FgRenderPass* render_pass, const PassResources& res) override;    
            void update_weights();
            void update_reproject_mat(sg::Scene* scene);
            private:
                static const glm::vec2 sHaltonSamples[16];
                TaaInfo _taa_info;
                std::shared_ptr<UniformBuffer> _taa_info_uniform_buf;
                rhi::TextureRef _history_img_view;
                glm::vec2 _frame_size;
                glm::vec2 _jitter;
                std::vector<glm::vec2> _sample_offsets;
                float _filter_width = 1.0f;
                glm::mat4 _pre_mat;
                bool _is_first_frame = true;

        };
    };

};
