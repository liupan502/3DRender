#include <core/renderers/sky_renderer.h>
#include <core/fg/fg_render_pass.h>
#include <core/image_view.h>
#include <core/sampler.h>

using namespace zr::core;

SkyRenderInfo::SkyRenderInfo() {
    const float EarthRayleighScaleHeight = 8.0f;
    rayleigh_info = glm::vec4(0.005802f, 0.013558f, 0.033100f, -1.0f / EarthRayleighScaleHeight);
    mie_scattering = glm::vec4(0.003996f, 0.003996f, 0.003996f, 0.0f);
    mie_extinction = glm::vec4(0.004440f, 0.004440f, 0.004440f, 0.0f);
    mie_absorption = mie_extinction - mie_scattering;

    absorption_density0 = glm::vec4(1.0f / 15.0f, -2.0f / 3.0f, 25.0f, 0.0f);

    absorption_density1 = glm::vec4(-1.0f / 15.0f, 8.0f / 3.0f, 0.0, 0.0);

    absorption_extinction = glm::vec4(0.000650f, 0.001881f, 0.000085f, 0.0f);

    sun_direction = glm::normalize(glm::vec4(-1.0, 0.0, 1.1, 0.0));

    camera_pos = glm::vec4(0.0f, 0.0f, 0.005f, 1.0f);

    group_albedo = glm::vec4(0.0f);

    bottom_radius = 6360.0f;

    top_radius = 6460.0f;

    const float EarthMieScaleHeight = 1.2f;
    
    mie_density_exp_scale = -1.0f / EarthMieScaleHeight;

    mie_phase_g = 0.8f;
}

void TransmittanceLutRenderer::prepare_desc(FgRenderPass* render_pass, const PassResources& res) {
    if (!_desc_set) {
        _pipeline = get_pipeline_mgr()->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
    }

    if (!_sky_render_info_uniform_buf) {
        _sky_render_info_uniform_buf = std::make_shared<UniformBuffer>(0, sizeof(SkyRenderInfo));
    }
    _sky_render_info_uniform_buf->update((const uint8_t*)(&_info), sizeof(SkyRenderInfo));
    _desc_set->update_desc_set_buffer({_sky_render_info_uniform_buf});

    reset_viewport(render_pass);
}

CreatePipelineFunc TransmittanceLutRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline> {
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/quad.vert"});  
        shader_path_map.insert({"frag", "shaders/sky_atmosphere_transmittance.frag"});

std::vector<rhi::DescriptorBindingInfo> binding_infos; 
        rhi::DescriptorBindingInfo binding_info0{};
        binding_info0.binding_idx = 0;
        binding_info0.desc_count = 1;
        binding_info0.desc_type = rhi::DescriptorType::DT_UNIFORM_BUFFER;
        binding_info0.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info0);

return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}

CreatePipelineFunc SkyViewLutRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline> {
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/quad.vert"});  
        shader_path_map.insert({"frag", "shaders/sky_atmosphere_sky_view.frag"});

        std::vector<rhi::DescriptorBindingInfo> binding_infos;

        rhi::DescriptorBindingInfo binding_info0{};
        binding_info0.binding_idx = 0;
        binding_info0.desc_count = 1;
        binding_info0.desc_type = rhi::DescriptorType::DT_UNIFORM_BUFFER;
        binding_info0.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info0);

        rhi::DescriptorBindingInfo binding_info1{};
        binding_info1.binding_idx = 1;
        binding_info1.desc_count = 1;
        binding_info1.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info1.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info1);

        return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}

void SkyViewLutRenderer::prepare_desc(FgRenderPass* render_pass, const PassResources& res) {
    if (!_desc_set) {
        _pipeline = get_pipeline_mgr()->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
    }

    if (!_sky_render_info_uniform_buf) {
        _sky_render_info_uniform_buf = std::make_shared<UniformBuffer>(0, sizeof(SkyRenderInfo));
    }
    _sky_render_info_uniform_buf->update((const uint8_t*)(&_info), sizeof(SkyRenderInfo));
    _desc_set->update_desc_set_buffer({_sky_render_info_uniform_buf});

    if (!res.input_textures.empty()) {
        if (!_sampler) {
            _sampler = rhi::rhi_instance->create_sample_state({});
        }
        _desc_set->update_desc_set_texture(_sampler, res.input_textures[0], 1);
    }

    reset_viewport(render_pass);  
}

