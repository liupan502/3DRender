#pragma once

#include <core/renderers/quad_renderer.h>

namespace zr
{
    namespace sg{
        class Texture;
    };
    namespace core {
        class ColorGradingRenderer : public QuadRenderer {
            public:
            virtual CreatePipelineFunc get_pipeline_creator() override;

            // dim_size should be 32;
            void set_color_grading_path(const std::string& path);

            protected:
            virtual void prepare_desc(FgRenderPass* render_pass) override;

            std::shared_ptr<sg::Texture> _color_grading_tex;

            std::string _color_grading_path;
        };
    };
} // namespace zr
