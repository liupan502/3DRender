#include <core/renderers/quad_renderer.h>
#include <core/pipeline_manager.h>
#include <core/descriptor.h>
#include <core/buffer.h>
#include <core/command_buffer.h>
#include <core/fg/fg_render_pass.h>
#include <core/sampler.h>
#include <scenegraph/components/material.h>
#include <fstream>

using namespace zr;
using namespace zr::core;

void QuadRenderer::prepare_renderpass(sg::Scene* scene, FgRenderPass* renderpass) {
    if (!_quad_rhi_buf) {
        auto size = 6 * sizeof(float);
        uint32_t stride = 0;
        rhi::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.stride = 0;
        bufferInfo.usage = static_cast<rhi::BufferUsageFlags>(rhi::BufferUsageFlagBit::VertexBuffer);
        _quad_rhi_buf = rhi::rhi_instance->create_buffer(bufferInfo);
        float data[6] = {-1.0f, -3.0f, -1.0f, 1.0f, 3.0f, 1.0f};
        rhi::rhi_instance->update_buffer(_quad_rhi_buf, data, size, 0);
    }

    if (!_quad_rhi_idx_buf) {
        rhi::BufferCreateInfo idx_ci{};
        idx_ci.size = 3 * sizeof(uint16_t);
        idx_ci.stride = 0;
        idx_ci.usage = rhi::BufferUsageFlagBit::IndexBuffer;
        _quad_rhi_idx_buf = rhi::rhi_instance->create_buffer(idx_ci);
        uint16_t idx_data[3] = {0, 1, 2};
        rhi::rhi_instance->update_buffer(_quad_rhi_idx_buf, idx_data, sizeof(idx_data), 0);
    }

    prepare_desc(renderpass);
}

    void QuadRenderer::reset_viewport(FgRenderPass *renderpass) {
    auto display_size = renderpass->get_display_size();
    _viewport = {0, 0,
                 (float)display_size.width, (float)display_size.height, 0, 1.0};
}

void QuadRenderer::prepare_desc(FgRenderPass* renderpass) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_available_desc_set();
    }

    auto view = renderpass->get_input_views()[0];
    rhi::SampleStateCreateInfo sampler_ci{};
    auto sampler = rhi::rhi_instance->create_sample_state(sampler_ci);
    _desc_set->update_desc_set_texture(sampler, view, 0);

    reset_viewport(renderpass);
}

void QuadRenderer::render_scene(sg::Scene* scene,
            const PassResources& res) {
    
    rhi::RenderPrimitive primitive{};
    primitive.vtx_buf = _quad_rhi_buf;
    primitive.idx_buf = _quad_rhi_idx_buf;

    rhi::rhi_instance->draw(_pipeline->get_rhi_pipeline(), primitive, 0, 3, 1);
}

QuadPipeline::QuadPipeline(
            std::shared_ptr<FgRenderPass> render_pass, uint32_t subpass_idx, 
            PipelineFeature feature
            , const std::map<std::string, std::string>& shader_path_map, 
                const std::vector<rhi::DescriptorBindingInfo>& binding_infos) : _shader_path_map(shader_path_map),
                Pipeline(feature) {

    _desc_layout = std::make_shared<DescriptorLayout>(binding_infos);
    create(render_pass, subpass_idx, _desc_layout);
}

void QuadPipeline::create_color_blend_state() {
    _ci.color_blend_attachments.clear();
    rhi::ColorBlendAttachmentState attachment{};
    attachment.blend_enable = false;
    attachment.color_write_mask = 0xF;
    _ci.color_blend_attachments.emplace_back(attachment);
}

void QuadPipeline::create_vtx_input_state() {
    _ci.vertex_bindings.clear();
    _ci.vertex_attributes.clear();

    rhi::VertexBindingDesc binding_desc{};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(float) * 2;
    binding_desc.input_rate = rhi::VertexInputRate::VIR_VERTEX;
    _ci.vertex_bindings.emplace_back(binding_desc);

    rhi::VertexAttributeDesc attr_desc{};
    attr_desc.location = 0;
    attr_desc.binding = 0;
    attr_desc.format = rhi::ColorFormat::R32G32_SFLOAT;
    attr_desc.offset = 0;
    _ci.vertex_attributes.emplace_back(attr_desc);
}

void QuadPipeline::create_depth_stencil_state() {
    _ci.depth_test_enable = false;
    _ci.depth_write_enable = false;
    _ci.depth_compare_op = rhi::CompareOp::CO_LESS;
}

void QuadPipeline::create_dynamic_states() {
    _dynamic_state_info = VkPipelineDynamicStateCreateInfo{};
    _dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    _dynamic_state_info.pNext = nullptr;
    _dynamic_state_info.flags = 0;
    _dynamic_states.clear();
    _dynamic_states.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    _dynamic_state_info.dynamicStateCount = _dynamic_states.size();
    _dynamic_state_info.pDynamicStates = _dynamic_states.data();
}

void QuadPipeline::create_shader_stage() {
    // vertex shader
    {
        std::ifstream file(_shader_path_map["vert"], std::ios::binary | std::ios::ate);
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> spirv(size);
        file.read(reinterpret_cast<char*>(spirv.data()), size);

        rhi::ShaderModuleCreateInfo ci{};
        ci.type = rhi::ShaderModuleType::SMT_VERTEX;
        ci.content = spirv.data();
        ci.len = static_cast<uint32_t>(size);
        ci.is_bin = true;

        _ci.vertex_shader = rhi::rhi_instance->create_shader_module(ci);
    }

    // fragment shader
    {
        std::ifstream file(_shader_path_map["frag"], std::ios::binary | std::ios::ate);
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> spirv(size);
        file.read(reinterpret_cast<char*>(spirv.data()), size);

        rhi::ShaderModuleCreateInfo ci{};
        ci.type = rhi::ShaderModuleType::SMT_FRAGMENT;
        ci.content = spirv.data();
        ci.len = static_cast<uint32_t>(size);
        ci.is_bin = true;

        _ci.fragment_shader = rhi::rhi_instance->create_shader_module(ci);
    }
}
    

CreatePipelineFunc QuadRenderer:: get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/spv/quad.vert.spv"});
        shader_path_map.insert({"frag", "shaders/spv/quad.frag.spv"}); 

        std::vector<rhi::DescriptorBindingInfo> binding_infos;
        rhi::DescriptorBindingInfo binding_info{};
        binding_info.binding_idx = 0;
        binding_info.desc_count = 1;
        binding_info.desc_type = rhi::DescriptorType::DT_SAMPLER_2D;
        binding_info.shader_stage = rhi::ShaderStageType::SST_FRAGMENT;
        binding_infos.emplace_back(binding_info);     

        return std::make_shared<QuadPipeline>(renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;
}
        

