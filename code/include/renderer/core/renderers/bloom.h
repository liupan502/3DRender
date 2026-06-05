#pragma once

#include <core/renderers/quad_renderer.h>

namespace zr {
    namespace core {
        class BloomDownSampleRenderer : public QuadRenderer {
            public:
            struct Parameters {
                float level;
                float threshold;
                float inv_highlight;
                Parameters() {
                    level = 0.0f;
                    threshold = 0.0f;
                    inv_highlight = 0.0f;
                };
            };

            public:
            virtual CreatePipelineFunc get_pipeline_creator() override;
            inline void set_parameters(const Parameters& params) {
                _params = params;
            }
            protected:
            virtual void prepare_desc(FgRenderPass* renderpass) override;

            protected:
            Parameters _params;
            std::shared_ptr<UniformBuffer> _param_uniform_buf;

        };

        class BloomUpSampleRenderer : public QuadRenderer {
            public:
            struct Parameters {
                glm::vec4 resolution;
                float  level;

                Parameters() {
                    resolution = glm::vec4(0.0f);
                    level = 0.0f;
                };
            };

            public:
            virtual CreatePipelineFunc get_pipeline_creator() override;

            inline void set_parameters(const Parameters& params) {
                _params = params;
            }

            protected:
            virtual void prepare_desc(FgRenderPass* renderpass) override;

            protected:
            Parameters _params;
            std::shared_ptr<UniformBuffer> _param_uniform_buf;
        };
        
    };
};


