#include <core/renderers/color_grading.h>
#include <core/fg/fg_render_pass.h>
#include <core/pipeline_manager.h>
#include <scenegraph/components/material.h>
#include <core/pipeline.h>
#include <core/sampler.h>
#include <core/image_view.h>
#include <scenegraph/components/texture.h>
#include <thread>
using namespace zr::core;

CreatePipelineFunc ColorGradingRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](std::shared_ptr<Device> device, PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline> {
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/color_grading.frag.spv"});   

        std::vector<DescriptorBindingInfo> binding_infos;   

        DescriptorBindingInfo binding_info_sample{};
        binding_info_sample.binding_idx = 0;
        binding_info_sample.desc_count = 1;
        binding_info_sample.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_sample.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_sample);

        DescriptorBindingInfo binding_info_sample1{};
        binding_info_sample1.binding_idx = 1;
        binding_info_sample1.desc_count = 1;
        binding_info_sample1.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_sample1.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_sample1);

        DescriptorBindingInfo binding_info_sample2{};
        binding_info_sample2.binding_idx = 2;
        binding_info_sample2.desc_count = 1;
        binding_info_sample2.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_sample2.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_sample2);

        return std::make_shared<QuadPipeline>(device, renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    
    return cp;
}

void ColorGradingRenderer::prepare_desc(FgRenderPass* render_pass, std::shared_ptr<Device> device) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_desc_pool()->get_available_desc_sets(1)[0];
        

    }

    if (_color_grading_path.size() >  0 && !_color_grading_tex) {
        _color_grading_tex = std::make_shared<sg::SingleLayerTexture>(_color_grading_path, 32, 32, 32, VK_FORMAT_R8G8B8A8_SRGB,
            sg::TextureSamplerType::TEXTURE_SAMPLER_3D);
        _color_grading_tex->upload_data(device);
    }

    auto view = render_pass->get_input_views()[0];
    std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>(device);
    _desc_set->update_desc_set_texture(sampler, view, 0);  
    
    auto view1 = render_pass->get_input_views()[1];
    std::shared_ptr<Sampler> sampler1 = std::make_shared<Sampler>(device);
    _desc_set->update_desc_set_texture(sampler1, view1, 1);
    
    _desc_set->update_desc_set_texture(_color_grading_tex, 2);

    reset_viewport(render_pass);
}

void ColorGradingRenderer::set_color_grading_path(const std::string& path) {
    _color_grading_path = path;
}
