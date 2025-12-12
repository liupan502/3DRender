//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>
#include <string>
#include <sstream>
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
    create_viewport_state(extent);
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
    _input_assembly_state = VkPipelineInputAssemblyStateCreateInfo{};
    _input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    _input_assembly_state.primitiveRestartEnable = VK_FALSE;
    _input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void Pipeline::create_depth_stencil_state() {
    _depth_stencil_state = VkPipelineDepthStencilStateCreateInfo{};
    _depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depth_stencil_state.stencilTestEnable = VK_FALSE;
    _depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    _depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
    _depth_stencil_state.depthTestEnable = _feature.get_material_ability().enable_depth;
    /*if (_feature.get_material_ability().enable_transparent) {
        _depth_stencil_state.depthWriteEnable = VK_FALSE;
    }
    else {
        _depth_stencil_state.depthWriteEnable = VK_TRUE;
    }*/

    _depth_stencil_state.depthWriteEnable = VK_TRUE;
    
    _depth_stencil_state.minDepthBounds = 0.0f;
    _depth_stencil_state.maxDepthBounds = 1.0f;
}

void Pipeline::create_rasterization_state() {
    _rasterization_state = VkPipelineRasterizationStateCreateInfo{};
    _rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _rasterization_state.depthBiasEnable = VK_FALSE;
    // _rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
    _rasterization_state.cullMode = (VkCullModeFlagBits)(_feature.get_material_ability().cull_mode);
    _rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    // _rasterization_state.polygonMode = VK_POLYGON_MODE_LINE;
    _rasterization_state.depthClampEnable = VK_FALSE;
    _rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE/*VK_FRONT_FACE_CLOCKWISE*/;
    _rasterization_state.lineWidth = 1;
}

void Pipeline::create_color_blend_state() {
    _color_blend_state = VkPipelineColorBlendStateCreateInfo{};
    _color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _color_blend_state.logicOpEnable = VK_FALSE;
    _vk_pipeline_color_blend_states.clear();
    for (uint8_t i = 0; i < 2; i++) {
        VkPipelineColorBlendAttachmentState vk_pipeline_color_blend_state = VkPipelineColorBlendAttachmentState{
            .blendEnable = VK_TRUE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
        if (_feature.get_material_ability().enable_transparent) {
            vk_pipeline_color_blend_state.blendEnable = VK_TRUE;
        }
        else {
            vk_pipeline_color_blend_state.blendEnable =
                            _feature.get_material_ability().enable_blend ? VK_TRUE : VK_FALSE;
                    
        }
        vk_pipeline_color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;
        vk_pipeline_color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
        vk_pipeline_color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        vk_pipeline_color_blend_state.dstColorBlendFactor =  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        vk_pipeline_color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        vk_pipeline_color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        _vk_pipeline_color_blend_states.emplace_back(vk_pipeline_color_blend_state);
    }
    
    _color_blend_state.pAttachments = _vk_pipeline_color_blend_states.data();
    _color_blend_state.attachmentCount = _vk_pipeline_color_blend_states.size();
}

void Pipeline::create_multisample_state(VkSampleCountFlagBits sample_count_flags_bits) {

    VkBool32 enable_multisample = VK_TRUE;
    if (sample_count_flags_bits == VK_SAMPLE_COUNT_1_BIT) {
        enable_multisample = VK_FALSE;
    }

    _multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _multisample_state.alphaToOneEnable = VK_FALSE;
    _multisample_state.alphaToCoverageEnable = VK_FALSE;
    _multisample_state.minSampleShading = 0;
    _multisample_state.sampleShadingEnable = VK_FALSE;
    _multisample_state.rasterizationSamples = sample_count_flags_bits;
    // VkSampleMask sample_mask = ~0u;
    _multisample_state.pSampleMask = &_vk_sample_mask;
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

void Pipeline::create_viewport_state(VkExtent2D extent) {
    _viewport_state = VkPipelineViewportStateCreateInfo{};
    _viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    _viewport_state.scissorCount = 1;
    
    _vk_scissor.offset.x = 0;
    _vk_scissor.offset.y = 0;
    _vk_scissor.extent = extent,

    _viewport_state.pScissors = &_vk_scissor;
    _viewport_state.viewportCount = 1;

    _vk_viewport.x = 0;
    _vk_viewport.y = 0;
    _vk_viewport.width = (float)extent.width;
    _vk_viewport.height = (float)extent.height;
    _vk_viewport.minDepth = 0.0f;
    _vk_viewport.maxDepth = 1.0f;

    _viewport_state.pViewports = &_vk_viewport;
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
    // _shader_modules.push_back(std::make_shared<ShaderModule>(_device, "shaders/base.vert.spv"));

    _shader_modules.push_back(std::make_shared<ShaderModule>(_device, "shaders/textured.vert", false, _feature.get_defines()));

    // fragment shader
    // _shader_modules.push_back(std::make_shared<ShaderModule>(_device, "shaders/base.frag.spv"));
    _shader_modules.push_back(std::make_shared<ShaderModule>(_device, "shaders/textured.frag", false, _feature.get_defines()));

    // std::vector<VkPipelineShaderStageCreateInfo> cis {};

    VkPipelineShaderStageCreateInfo vtx_ci{};
    vtx_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vtx_ci.module = _shader_modules[0]->get();
    vtx_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vtx_ci.pName = "main";
    _shader_cis.push_back(vtx_ci);

    VkPipelineShaderStageCreateInfo frag_ci{};
    frag_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_ci.module = _shader_modules[1]->get();
    frag_ci.pName = "main";
    frag_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    _shader_cis.push_back(frag_ci);
}

VkFormat get_data_format(zr::sg::VertexAttributeType type) {
    VkFormat fmt;
    switch (type) {
        case zr::sg::VERTEX_ATTRI_POS:
            fmt = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_UV:
            fmt = VK_FORMAT_R32G32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_COLOR:
            fmt = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_NORMAL:
            fmt = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case zr::sg::VERTEX_ATTRI_JOINT_8:
            fmt = VK_FORMAT_R8G8B8_UINT;
            break;
        case zr::sg::VERTEX_ATTRI_JOINT_16:
            fmt = VK_FORMAT_R16G16B16A16_UINT;
            break;
        case zr::sg::VERTEX_ATTRI_WEIGHT:
            fmt = VK_FORMAT_R32G32B32A32_SFLOAT;
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




