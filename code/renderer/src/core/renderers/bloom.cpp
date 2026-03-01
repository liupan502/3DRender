#include <core/renderers/bloom.h>
#include <core/fg/fg_render_pass.h>
#include <core/sampler.h>
#include <scenegraph/components/material.h>
#include <core/buffer.h>

using namespace zr::core;

CreatePipelineFunc BloomDownSampleRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](std::shared_ptr<Device> device, PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/bloom_downsample.frag.spv"});   

        std::vector<DescriptorBindingInfo> binding_infos;   

        DescriptorBindingInfo binding_info_sample{};
        binding_info_sample.binding_idx = 50;
        binding_info_sample.desc_count = 1;
        binding_info_sample.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_sample.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_sample);

        DescriptorBindingInfo binding_info_params{};
        binding_info_params.binding_idx = 51;
        binding_info_params.desc_count = 1;
        binding_info_params.desc_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding_info_params.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_params);

        return std::make_shared<QuadPipeline>(device, renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}

void BloomDownSampleRenderer::prepare_desc(FgRenderPass* renderpass, std::shared_ptr<Device> device) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_desc_pool()->get_available_desc_sets(1)[0];
        
        _param_uniform_buf = std::make_shared<UniformBuffer>(51, device, sizeof(Parameters)); 
    }

    auto view = renderpass->get_input_views()[0];
    SamplerInfo si;
    si.amu = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    si.amv = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>(device, si);
    _desc_set->update_desc_set_texture(sampler, view, 50);  
    
    _param_uniform_buf->update((uint8_t*)(&_params), sizeof(Parameters));
    _desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<UniformBuffer>>({_param_uniform_buf}), 1);

    // _desc_set->update_desc_set_buffer()
    reset_viewport(renderpass);
}

CreatePipelineFunc BloomUpSampleRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](std::shared_ptr<Device> device, PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/bloom_upsample.frag.spv"});   

        std::vector<DescriptorBindingInfo> binding_infos;   

        DescriptorBindingInfo binding_info_sample{};
        binding_info_sample.binding_idx = 60;
        binding_info_sample.desc_count = 1;
        binding_info_sample.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_sample.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_sample);

        DescriptorBindingInfo binding_info_params{};
        binding_info_params.binding_idx = 61;
        binding_info_params.desc_count = 1;
        binding_info_params.desc_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding_info_params.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_params);

        return std::make_shared<QuadPipeline>(device, renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}

void BloomUpSampleRenderer::prepare_desc(FgRenderPass* renderpass, std::shared_ptr<Device> device) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_desc_pool()->get_available_desc_sets(1)[0];
        
        _param_uniform_buf = std::make_shared<UniformBuffer>(61, device, sizeof(Parameters)); 
    }

    auto view = renderpass->get_input_views()[0];
    SamplerInfo si;
    si.amu = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    si.amv = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>(device, si);

    _desc_set->update_desc_set_texture(sampler, view, 60);  
    
    _param_uniform_buf->update((uint8_t*)(&_params), sizeof(Parameters));
    _desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<UniformBuffer>>({_param_uniform_buf}), 1);
    reset_viewport(renderpass);
}