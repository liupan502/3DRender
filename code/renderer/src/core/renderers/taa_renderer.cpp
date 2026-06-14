#include <core/renderers/taa_renderer.h>
#include <core/fg/fg_render_pass.h>
#include <core/image_view.h>
#include <core/sampler.h>
#include <scenegraph/scene_graph.h>

using namespace zr::core;

TaaRenderer::TaaRenderer() {
    _sample_offsets = {
        { -1.0f, -1.0f }, {  0.0f, -1.0f }, {  1.0f, -1.0f },
        { -1.0f,  0.0f }, {  0.0f,  0.0f }, {  1.0f,  0.0f },
        { -1.0f,  1.0f }, {  0.0f,  1.0f }, {  1.0f,  1.0f }
    };
}

static float halton_kernel(uint16_t i, uint16_t b) {
    i += 409;
    float f = 1.0f;
    float r = 0.0f;
    while (i > 0u) {
        f /= float(b);
        r += f * float(i % b);
        i /= b;
    }
    return r;
}

const glm::vec2 TaaRenderer::sHaltonSamples[16] = {
        { halton_kernel( 0, 2), halton_kernel( 0, 3) },
        { halton_kernel( 1, 2), halton_kernel( 1, 3) },
        { halton_kernel( 2, 2), halton_kernel( 2, 3) },
        { halton_kernel( 3, 2), halton_kernel( 3, 3) },
        { halton_kernel( 4, 2), halton_kernel( 4, 3) },
        { halton_kernel( 5, 2), halton_kernel( 5, 3) },
        { halton_kernel( 6, 2), halton_kernel( 6, 3) },
        { halton_kernel( 7, 2), halton_kernel( 7, 3) },
        { halton_kernel( 8, 2), halton_kernel( 8, 3) },
        { halton_kernel( 9, 2), halton_kernel( 9, 3) },
        { halton_kernel(10, 2), halton_kernel(10, 3) },
        { halton_kernel(11, 2), halton_kernel(11, 3) },
        { halton_kernel(12, 2), halton_kernel(12, 3) },
        { halton_kernel(13, 2), halton_kernel(13, 3) },
        { halton_kernel(14, 2), halton_kernel(14, 3) },
        { halton_kernel(15, 2), halton_kernel(15, 3) }
};

glm::vec2 TaaRenderer::halton(uint8_t idx) {
    return sHaltonSamples[idx & 0xFu];
}

CreatePipelineFunc TaaRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) ->std::shared_ptr<Pipeline> {
        
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/taa.frag.spv"});

        std::vector<rhi::DescriptorBindingInfo> binding_infos;

        rhi::DescriptorBindingInfo binding_info_color_sample{};
        binding_info_color_sample.binding_idx = 0;
        binding_info_color_sample.desc_count = 1;
        binding_info_color_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_color_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_color_sample);

        rhi::DescriptorBindingInfo binding_info_depth_sample{};
        binding_info_depth_sample.binding_idx = 1;
        binding_info_depth_sample.desc_count = 1;
        binding_info_depth_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_depth_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_depth_sample);    

        rhi::DescriptorBindingInfo binding_info_history_sample{};
        binding_info_history_sample.binding_idx = 2;
        binding_info_history_sample.desc_count = 1;
        binding_info_history_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_history_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_history_sample);  

        rhi::DescriptorBindingInfo binding_info_taa_info_uniform{};
        binding_info_taa_info_uniform.binding_idx = 3;
        binding_info_taa_info_uniform.desc_count = 1;
        binding_info_taa_info_uniform.desc_type = rhi::DescriptorType::DT_UNIFORM_BUFFER;
        binding_info_taa_info_uniform.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_taa_info_uniform);

        rhi::DescriptorBindingInfo binding_info_velocity_sample{};
        binding_info_velocity_sample.binding_idx = 4;
        binding_info_velocity_sample.desc_count = 1;
        binding_info_velocity_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_velocity_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_velocity_sample);  

        return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };


    return cp;                 
}

void TaaRenderer::prepare_desc(FgRenderPass* render_pass, const PassResources& res) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
    }

    if (!_taa_info_uniform_buf) {
        _taa_info_uniform_buf = std::make_shared<UniformBuffer>(3, sizeof(TaaInfo));
    }

    auto sampler = rhi::rhi_instance->create_sample_state({});
    if (res.input_textures.size() > 0) {
        _desc_set->update_desc_set_texture(sampler, res.input_textures[0], 0);
    }

    // auto depth_view = res.input_textures.size() > 1 ? res.input_textures[1] : nullptr;
    // _desc_set->update_desc_set_texture(sampler, depth_view, 1);

    auto view = res.input_textures.size() > 0 ? res.input_textures[0] : nullptr;
    auto history = _history_img_view ? _history_img_view : view;
    // history = view;
    if (history) {
        _desc_set->update_desc_set_texture(sampler, history, 2);
    }

    if (res.input_textures.size() > 1) {
        _desc_set->update_desc_set_texture(sampler, res.input_textures[1], 4);
    }

    _taa_info.alpha = 0.05f;

    update_weights();
    auto color_outputs = render_pass->get_color_output_textures();
    if (color_outputs.size() > 0) {
        _history_img_view = color_outputs[0];
    }

    reset_viewport(render_pass);
}

void TaaRenderer::render_scene(sg::Scene* scene,
            const PassResources& res) {
    update_reproject_mat(scene);
    _taa_info_uniform_buf->update((const uint8_t*)(&_taa_info), sizeof(TaaInfo));
    _desc_set->update_desc_set_buffer({_taa_info_uniform_buf});

    QuadRenderer::render_scene(scene, res);

    if (_is_first_frame) {
        _is_first_frame = false;
    }
}

void TaaRenderer::update_weights() {

    float sum = 0.0f;
    for (uint8_t i = 0; i < 9; i++) {
        auto d = _sample_offsets[i] - _jitter;
        d = d * (1.0f / _filter_width);
        _taa_info.filter_weights[i * 4] = std::exp2(-3.3f * (d.x * d.x + d.y * d.y));
        sum += _taa_info.filter_weights[i * 4];
    }            

    for (uint8_t i = 0; i < 9; i++) {
        _taa_info.filter_weights[i * 4] /= sum;
    }
}

void TaaRenderer::update_reproject_mat(sg::Scene* scene) {

    std::shared_ptr<sg::Camera> camera = scene->get_active_camera();
    auto current_mat = camera->get_projection() * camera->get_view();

    glm::mat4 scale_mat(1.0f);
    scale_mat[0][0] = _frame_size.x / _viewport.width;
    scale_mat[1][1] = _frame_size.y / _viewport.height;

    glm::mat4 reproject_mat{
                        0.5, 0, 0, 0,
                        0, 0.5, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1
    };

    if (!_is_first_frame) {
        // auto tmp = reproject_mat * scale_mat;
        // reproject_mat =glm::inverse(tmp) * _pre_mat * glm::inverse(current_mat)  * tmp;
        // reproject_mat = reproject_mat * scale_mat;
    }

    _taa_info.reproject = reproject_mat * scale_mat;
    _pre_mat = current_mat;
}
