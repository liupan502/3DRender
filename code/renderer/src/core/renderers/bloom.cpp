#include <core/renderers/bloom.h>
#include <core/fg/fg_render_pass.h>
#include <core/sampler.h>
#include <scenegraph/components/material.h>
#include <core/buffer.h>
#include <rhi/rhi.h>

using namespace zr::core;

CreatePipelineFunc BloomDownSampleRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/bloom_downsample.frag.spv"});   

        std::vector<rhi::DescriptorBindingInfo> binding_infos;   

        rhi::DescriptorBindingInfo binding_info_sample{};
        binding_info_sample.binding_idx = 50;
        binding_info_sample.desc_count = 1;
        binding_info_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_sample);

        rhi::DescriptorBindingInfo binding_info_params{};
        binding_info_params.binding_idx = 51;
        binding_info_params.desc_count = 1;
        binding_info_params.desc_type = rhi::DescriptorType::DT_UNIFORM_BUFFER;
        binding_info_params.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_params);

        return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}

void BloomDownSampleRenderer::prepare_desc(FgRenderPass* renderpass, const PassResources& res) {
    if (!_desc_set) {
        _pipeline = get_pipeline_mgr()->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
        
        _param_uniform_buf = std::make_shared<UniformBuffer>(51, sizeof(Parameters)); 
    }

    if (!res.input_textures.empty()) {
        rhi::SampleStateCreateInfo sampler_ci{};
        sampler_ci.address_u = rhi::SAM_CLAMP_EDGE;
        sampler_ci.address_v = rhi::SAM_CLAMP_EDGE;
        auto sampler = rhi::rhi_instance->create_sample_state(sampler_ci);
        _desc_set->update_desc_set_texture(sampler, res.input_textures[0], 50);
    }
    
    _param_uniform_buf->update((uint8_t*)(&_params), sizeof(Parameters));
    _desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<UniformBuffer>>({_param_uniform_buf}), 1);

    reset_viewport(renderpass);
}

CreatePipelineFunc BloomUpSampleRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/bloom_upsample.frag.spv"});   

        std::vector<rhi::DescriptorBindingInfo> binding_infos;   

        rhi::DescriptorBindingInfo binding_info_sample{};
        binding_info_sample.binding_idx = 60;
        binding_info_sample.desc_count = 1;
        binding_info_sample.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info_sample.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_sample);

        rhi::DescriptorBindingInfo binding_info_params{};
        binding_info_params.binding_idx = 61;
        binding_info_params.desc_count = 1;
        binding_info_params.desc_type = rhi::DescriptorType::DT_UNIFORM_BUFFER;
        binding_info_params.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info_params);

        return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}

void BloomUpSampleRenderer::prepare_desc(FgRenderPass* renderpass, const PassResources& res) {
    if (!_desc_set) {
        _pipeline = get_pipeline_mgr()->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
        
        _param_uniform_buf = std::make_shared<UniformBuffer>(61, sizeof(Parameters)); 
    }

    if (!res.input_textures.empty()) {
        rhi::SampleStateCreateInfo sampler_ci{};
        sampler_ci.address_u = rhi::SAM_CLAMP_EDGE;
        sampler_ci.address_v = rhi::SAM_CLAMP_EDGE;
        auto sampler = rhi::rhi_instance->create_sample_state(sampler_ci);
        _desc_set->update_desc_set_texture(sampler, res.input_textures[0], 60);
    }
    
    _param_uniform_buf->update((uint8_t*)(&_params), sizeof(Parameters));
    _desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<UniformBuffer>>({_param_uniform_buf}), 1);
    reset_viewport(renderpass);
}