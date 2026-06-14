#pragma once
#include <core/renderers/quad_renderer.h>

namespace zr {
    namespace core {

        struct SkyRenderInfo{
            SkyRenderInfo();

            // xyz for exp scale, w for scattering
            glm::vec4 rayleigh_info;
            
            glm::vec4 mie_scattering;

            glm::vec4 mie_extinction;

            glm::vec4 mie_absorption;

            // x for constant, y for linear, z for layer width
            glm::vec4 absorption_density0;

            glm::vec4 absorption_density1;

            glm::vec4 absorption_extinction;

            glm::vec4 group_albedo;

            glm::vec4 sun_direction;

            glm::vec4 camera_pos;

            float bottom_radius;
            float top_radius;
            float mie_density_exp_scale;
            float mie_phase_g;
        };

        class TransmittanceLutRenderer : public QuadRenderer {
            public:
            virtual CreatePipelineFunc get_pipeline_creator() override;
            inline void set_sky_render_info(const SkyRenderInfo& info) { 
                _info = info;
            }
            protected:
            virtual void prepare_desc(FgRenderPass* render_pass, const PassResources& res) override;

            protected:
            SkyRenderInfo _info;
            std::shared_ptr<UniformBuffer> _sky_render_info_uniform_buf;
        };

        class SkyViewLutRenderer : public QuadRenderer {
            public:
            virtual CreatePipelineFunc get_pipeline_creator() override;
            inline void set_sky_render_info(const SkyRenderInfo& info) {
                _info = info;
            }
            protected:
            virtual void prepare_desc(FgRenderPass* render_pass, const PassResources& res) override;

            protected:
            SkyRenderInfo _info;
            std::shared_ptr<UniformBuffer> _sky_render_info_uniform_buf;
        };

        class AerialPerspectiveLutRenderer : public QuadRenderer {

        };

        class MultipleScatteringLutRenderer : public QuadRenderer {

        };

        class SkyRenderer : public QuadRenderer {

        };
    };
};