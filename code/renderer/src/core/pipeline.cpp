//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>
#include <string>
#include <sstream>
#include <fstream>
#include <scenegraph/components/material.h>
#include <scenegraph/components/mesh.h>
#include <core/fg/fg_render_pass.h>
#include <utils/log.h>

// #include "../../vulkan/vulkan_util.h"
using namespace zr::core;

Pipeline::Pipeline(std::shared_ptr<Device> device, PipelineFeature feature) : _device(device),
                                                                              _feature(feature) {
}

void Pipeline::create_vtx_input_state() {

}

void Pipeline::create_shader_stage(const std::map<VkShaderStageFlagBits, std::string>& shader_map) {
    
}

bool Pipeline::create(std::shared_ptr<FgRenderPass> fg_render_pass, 
                      uint32_t subpass_idx,
                      std::shared_ptr<PipelineLayout> layout) {
    VkGraphicsPipelineCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // shader stages
    this->create_shader_stage();
    ci.stageCount = _shader_cis.size();
    ci.pStages = _shader_cis.data();

    // viewport state
    VkExtent2D extent = fg_render_pass->get_display_size();
    create_viewport_state(glm::vec2(static_cast<float>(extent.width), static_cast<float>(extent.height)));
    ci.pViewportState = &_viewport_state;

    // multisample state
    create_multisample_state(fg_render_pass->get_sample_count());
    if (false) {
        ci.pMultisampleState = nullptr;
    }
    else {
        ci.pMultisampleState = &_multisample_state;
    }


    // color blend state
    create_color_blend_state();
    ci.pColorBlendState = &_color_blend_state;

    // rasterization state
    create_rasterization_state();
    ci.pRasterizationState = &_rasterization_state;

    // input assembly state
    create_input_assembly_state();
    ci.pInputAssemblyState = &_input_assembly_state;

    // vertex input state
    create_vtx_input_state();
    ci.pVertexInputState = &_vtx_input_state;

    // depeth stencil state
    create_depth_stencil_state();
    ci.pDepthStencilState = &_depth_stencil_state;

    create_dynamic_states();
    ci.pDynamicState = &_dynamic_state_info;
    // ci.pDynamicState = nullptr;

    // pipeline create pipeline layout
    ci.layout = layout->get();

    ci.renderPass = fg_render_pass->get_render_pass()->get();
    ci.subpass = subpass_idx;
    ci.basePipelineHandle = VK_NULL_HANDLE;
    ci.basePipelineIndex = 0;

    // pipeline cache
    create_pipeline_cache();
    VkResult ret = vkCreateGraphicsPipelines(_device->get_device(), _vk_pipeline_cache, 1, &ci, nullptr, &_vk_pipeline);
    // CALL_VK(vkCreateGraphicsPipelines(_device->get_device(), _vk_pipeline_cache, 1, &ci, nullptr, &_vk_pipeline));
    if (ret != VK_SUCCESS) {
        LOGD("create graphic pipeline failed");
    }
    else {
        LOGD("create graphic pipeline success");
    }
    return true;
}

void Pipeline::create_input_assembly_state() {
    _ci.topology = rhi::PrimitiveTopology::PT_TRIANGLE_LIST;
    _ci.primitive_restart_enable = false;
}

void Pipeline::create_depth_stencil_state() {
    _ci.depth_test_enable = _feature.get_material_ability().enable_depth;
    _ci.depth_write_enable = true;
    _ci.depth_compare_op = rhi::CompareOp::CO_LESS;
}

void Pipeline::create_rasterization_state() {
    _ci.polygon_mode = rhi::PolygonMode::PM_FILL;
    _ci.cull_mode = static_cast<rhi::CullMode>(_feature.get_material_ability().cull_mode);
    _ci.front_face = rhi::FrontFace::FF_COUNTER_CLOCKWISE;
    _ci.depth_clamp_enable = false;
    _ci.line_width = 1.0f;
}

void Pipeline::create_color_blend_state() {
    _ci.color_blend_attachments.clear();
    for (uint8_t i = 0; i < 2; i++) {
        rhi::ColorBlendAttachmentState attachment{};
        attachment.color_write_mask = 0xF;
        if (_feature.get_material_ability().enable_transparent) {
            attachment.blend_enable = true;
        }
        else {
            attachment.blend_enable = _feature.get_material_ability().enable_blend;
        }
        attachment.color_blend_op = rhi::BlendOp::BO_ADD;
        attachment.alpha_blend_op = rhi::BlendOp::BO_ADD;
        attachment.dst_alpha_factor = rhi::BlendFactor::BF_ONE;
        attachment.dst_color_factor = rhi::BlendFactor::BF_ONE_MINUS_SRC_ALPHA;
        attachment.src_alpha_factor = rhi::BlendFactor::BF_ZERO;
        attachment.src_color_factor = rhi::BlendFactor::BF_SRC_ALPHA;
        _ci.color_blend_attachments.emplace_back(attachment);
    }
}

void Pipeline::create_multisample_state(rhi::SampleCount sample_count) {
    _ci.sample_count = sample_count;
}

void Pipeline::create_dynamic_states() {
    _dynamic_state_info = VkPipelineDynamicStateCreateInfo{};
    _dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    _dynamic_state_info.pNext = nullptr;
    _dynamic_state_info.flags = 0;
    _dynamic_states.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    _dynamic_state_info.dynamicStateCount = _dynamic_states.size();
    _dynamic_state_info.pDynamicStates = _dynamic_states.data();
}

void Pipeline::create_viewport_state(glm::vec2 display_size) {
    _ci.viewport.left = 0;
    _ci.viewport.top = 0;
    _ci.viewport.width = static_cast<uint32_t>(display_size.x);
    _ci.viewport.height = static_cast<uint32_t>(display_size.y);
}

void Pipeline::create_pipeline_cache() {
    VkPipelineCacheCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    ci.pInitialData = nullptr;
    ci.initialDataSize = 0;
    CALL_VK(vkCreatePipelineCache(_device->get_device(), &ci, nullptr, &_vk_pipeline_cache));
}

void BasePipeline::create_shader_stage() {
    // vertex shader
    {
        std::ifstream file("shaders/textured.vert.spv", std::ios::binary | std::ios::ate);
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
        std::ifstream file("shaders/textured.frag.spv", std::ios::binary | std::ios::ate);
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

rhi::ColorFormat get_data_format(zr::sg::VertexAttributeType type) {
    rhi::ColorFormat fmt = rhi::ColorFormat::None;
    switch (type) {
        case zr::sg::VERTEX_ATTRI_POS:
            fmt = rhi::ColorFormat::R32G32B32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_UV:
            fmt = rhi::ColorFormat::R32G32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_COLOR:
            fmt = rhi::ColorFormat::R32G32B32A32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_NORMAL:
            fmt = rhi::ColorFormat::R32G32B32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_JOINT_8:
            fmt = rhi::ColorFormat::R8G8B8_UINT ;
            break;
        case zr::sg::VERTEX_ATTRI_JOINT_16:
            fmt = rhi::ColorFormat::R16G16B16A16_UINT;
            break;
        case zr::sg::VERTEX_ATTRI_WEIGHT:
            fmt = rhi::ColorFormat::R32G32B32A32_SFLOAT;
            break;
        default:
            break;
    }
    return fmt;
}

void BasePipeline::create_vtx_input_state() {

    _vk_input_binding_descs.clear();
    _vk_input_attri_descs.clear();
    auto attrs = _feature.get_attrs();
    for (uint32_t i = 0; i < attrs.size(); i++) {
        VkVertexInputBindingDescription desc = VkVertexInputBindingDescription{
                .binding = i,
                .stride = attrs[i][0].stride,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        _vk_input_binding_descs.push_back(desc);

        for (uint32_t j = 0; j < attrs[i].size(); j++) {
            auto attr = attrs[i][j];
            _vk_input_attri_descs.emplace_back(VkVertexInputAttributeDescription{
                    .location = attr.location,
                    .binding = i,
                    .format = get_data_format(attr.type),
                    .offset = attr.offset,
            });
        }
    }

    _vtx_input_state =   VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(_vk_input_binding_descs.size()),
            .pVertexBindingDescriptions = _vk_input_binding_descs.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(_vk_input_attri_descs.size()),
            .pVertexAttributeDescriptions = _vk_input_attri_descs.data(),
    };
}

void BasePipeline::create_pipeline_layout() {
    // _layout = std::make_shared<PipelineLayout>()
}

BasePipeline::BasePipeline(std::shared_ptr<Device> device,
        std::shared_ptr<FgRenderPass> fg_render_pass, 
        uint32_t subpass_idx,PipelineFeature feature) : Pipeline(device, feature) {
    _desc_layout = std::make_shared<BaseDescriptorLayout>(device, _feature);
    _desc_pool = std::make_shared<DescriptorPool>(device, _desc_layout, 500);
    _layout = std::make_shared<PipelineLayout>(_device, _desc_layout);
    create(fg_render_pass, subpass_idx, _layout);
}

void Pipeline::bind(std::shared_ptr<CommandBuffer> cmd_buf) {
    vkCmdBindPipeline(cmd_buf->get(), VK_PIPELINE_BIND_POINT_GRAPHICS, _vk_pipeline);
}

PipelineLayout::PipelineLayout(std::shared_ptr<Device> device,
                               std::shared_ptr<DescriptorLayout> desc_set_layout) : _device(device){
    VkDescriptorSetLayout vk_desc_set_layout = desc_set_layout->get();
    VkPipelineLayoutCreateInfo pipeline_layout_ci{};
    pipeline_layout_ci.flags = 0;
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = nullptr;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &vk_desc_set_layout;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    CALL_VK(vkCreatePipelineLayout(_device->get_device(), &pipeline_layout_ci, nullptr, &_vk_pipeline_layout));
}

Pipeline::~Pipeline() {
    vkDestroyPipelineCache(_device->get_device(), _vk_pipeline_cache, nullptr);
    vkDestroyPipeline(_device->get_device(), _vk_pipeline, nullptr);
}

PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(_device->get_device(), _vk_pipeline_layout, nullptr);
}

uint16_t LightInfo::get_val() {
    uint16_t val = directional_light_count ;  // max for 5
    val = val << 3 + point_light_count;      // max for 3
    val = val << 2 + spot_light_count;       // max for 3
    val = val << 2 + environment_light_count;// max for 1
    return val;
}

uint64_t PipelineFeature::get_val() {
    uint64_t val = _light_info.get_val();
    val = (val << 32) + _material_ability.get_val();
    return val;
}

std::vector<std::string> PipelineFeature::get_defines() const {
    std::vector<std::string> defines;
    defines.emplace_back("#version 450\n");
    std::stringstream ss;
    if (_light_info.point_light_count > 0) {
        ss.str("");
        ss << "#define POINT_LIGHT_COUNT " << _light_info.point_light_count << std::endl;
        defines.emplace_back(ss.str());
    }

    if (_light_info.directional_light_count > 0) {
        ss.str("");
        ss << "#define DIRECTIONAL_LIGHT_COUNT " << _light_info.directional_light_count << std::endl;
        defines.emplace_back(ss.str());
    }

    if (_light_info.spot_light_count > 0) {
        ss.str("");
        ss << "#define SPOT_LIGHT_COUNT " << _light_info.spot_light_count << std::endl;
        defines.emplace_back(ss.str());
    }

    if (_material_ability.enable_skin) {
        ss.str("");
        ss << "#define  SKINNING" << std::endl;
        defines.emplace_back(ss.str());
    }

    if (_material_ability.enable_environment && _light_info.environment_light_count > 0) {
        ss.str("");
        ss << "#define  ENVIRONMENT_LIGHT" << std::endl;
        defines.emplace_back(ss.str());
    }

    if (_material_ability.enable_color) {
        ss.str("");
        ss << "#define  VERTEX_COLOR" << std::endl;
        defines.emplace_back(ss.str());
    }
    return defines;
}

bool PipelineFeature::PipelineFeatureComp::operator()(const PipelineFeature &lhs,
                                                      const PipelineFeature &rhs) const {
    if (lhs.get_light_info().get_val() < rhs.get_light_info().get_val()) {
        return true;
    }
    else if (lhs.get_light_info().get_val() > rhs.get_light_info().get_val()) {
        return false;
    }

    if (lhs._material_ability.get_val() < rhs._material_ability.get_val()) {
        return true;
    }
    else if (lhs._material_ability.get_val() > rhs._material_ability.get_val()) {
        return false;
    }

    if (lhs.get_attrs() < rhs.get_attrs()) {
        return true;
    }
    else if (lhs.get_attrs() > rhs.get_attrs()) {
        return false;
    }

    return false;
}

bool LightInfo::has_light() {
    return (directional_light_count > 0) || (spot_light_count > 0) || (point_light_count > 0);
}

PipelineFeature::PipelineFeature(LightInfo light_info, std::shared_ptr<sg::Material> material,
                std::vector<std::vector<sg::VertexAttribute>> vtx_attrs) :
                            _light_info(light_info), _vtx_attrs(vtx_attrs){
    if (material) {
        _material_ability = material->get_ability();
    }
}




