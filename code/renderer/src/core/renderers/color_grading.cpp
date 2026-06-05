#include <core/renderers/color_grading.h>
#include <core/fg/fg_render_pass.h>
#include <core/pipeline_manager.h>
#include <scenegraph/components/material.h>
#include <core/pipeline.h>
#include <core/image_view.h>
#include <scenegraph/components/texture.h>
#include <thread>
using namespace zr::core;

CreatePipelineFunc ColorGradingRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline> {
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/color_grading.frag.spv"});   

        std::vector<rhi::DescriptorBindingInfo> binding_infos;   

        rhi::DescriptorBindingInfo binding_info_sample{};
        binding_info_sample.binding_idx = 0;
        binding_info_sample.desc_count = 1;
        binding_info_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_sample);

        rhi::DescriptorBindingInfo binding_info_sample1{};
        binding_info_sample1.binding_idx = 1;
        binding_info_sample1.desc_count = 1;
        binding_info_sample1.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_sample1.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_sample1);

        rhi::DescriptorBindingInfo binding_info_sample2{};
        binding_info_sample2.binding_idx = 2;
        binding_info_sample2.desc_count = 1;
        binding_info_sample2.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_sample2.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_sample2);

        return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    
    return cp;
}

void ColorGradingRenderer::prepare_desc(FgRenderPass* render_pass) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
    }

    if (_color_grading_path.size() >  0 && !_color_grading_tex) {
        _color_grading_tex = std::make_shared<sg::SingleLayerTexture>(_color_grading_path, 32, 32, 32, VK_FORMAT_R8G8B8A8_SRGB,
            sg::TextureSamplerType::TEXTURE_SAMPLER_3D);
        
        _color_grading_tex->upload_data();
    }

    auto view = render_pass->get_input_views()[0];
    auto sampler = rhi::rhi_instance->create_sample_state({});
    _desc_set->update_desc_set_texture(sampler, view, 0);
    
    auto view1 = render_pass->get_input_views()[1];
    auto sampler1 = rhi::rhi_instance->create_sample_state({});
    _desc_set->update_desc_set_texture(sampler1, view1, 1);
    
    _desc_set->update_desc_set_texture(_color_grading_tex, 2);

    reset_viewport(render_pass);
}

void ColorGradingRenderer::set_color_grading_path(const std::string& path) {
    _color_grading_path = path;
}
