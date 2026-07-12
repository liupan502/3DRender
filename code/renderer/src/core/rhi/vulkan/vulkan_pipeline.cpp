#include <rhi/vulkan/vulkan_pipeline.h>
#include <rhi/vulkan/vulkan_shader_module.h>
#include <rhi/vulkan/vulkan_descriptor.h>
#include <core/device.h>
#include <core/vk_common.h>

namespace rhi {
namespace vulkan {

static VkFormat color_format_to_vk(ColorFormat fmt) {
    switch (fmt) {
        case ColorFormat::R8G8B8A8_UNORM:   return VK_FORMAT_R8G8B8A8_UNORM;
        case ColorFormat::R8G8B8A8_SRGB:    return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::R8G8B8A8_SNORM:   return VK_FORMAT_R8G8B8A8_SNORM;
        case ColorFormat::R8G8B8A8_UINT:    return VK_FORMAT_R8G8B8A8_UINT;
        case ColorFormat::R8G8B8A8_SINT:    return VK_FORMAT_R8G8B8A8_SINT;
        case ColorFormat::R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ColorFormat::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ColorFormat::R32G32_SFLOAT:    return VK_FORMAT_R32G32_SFLOAT;
        case ColorFormat::R32G32B32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
        case ColorFormat::R16G16_SFLOAT:    return VK_FORMAT_R16G16_SFLOAT;
        case ColorFormat::B10G11R11_UFLOAT_PACK32: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case ColorFormat::D32_SFLOAT:       return VK_FORMAT_D32_SFLOAT;
        case ColorFormat::B8G8R8A8_SRGB:    return VK_FORMAT_B8G8R8A8_SRGB;
        case ColorFormat::R8G8B8_UINT:      return VK_FORMAT_R8G8B8_UINT;
        case ColorFormat::R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
        default:                            return VK_FORMAT_UNDEFINED;
    }
}

static VkPrimitiveTopology topology_to_vk(PrimitiveTopology t) {
    switch (t) {
        case PT_POINT_LIST:       return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PT_LINE_LIST:        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PT_LINE_STRIP:       return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PT_TRIANGLE_LIST:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PT_TRIANGLE_STRIP:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PT_TRIANGLE_FAN:     return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:                  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

static VkPolygonMode polygon_mode_to_vk(PolygonMode m) {
    switch (m) {
        case PM_FILL:  return VK_POLYGON_MODE_FILL;
        case PM_LINE:  return VK_POLYGON_MODE_LINE;
        case PM_POINT: return VK_POLYGON_MODE_POINT;
        default:       return VK_POLYGON_MODE_FILL;
    }
}

static VkCullModeFlags cull_mode_to_vk(CullMode m) {
    switch (m) {
        case CM_NONE:           return VK_CULL_MODE_NONE;
        case CM_FRONT:          return VK_CULL_MODE_FRONT_BIT;
        case CM_BACK:           return VK_CULL_MODE_BACK_BIT;
        case CM_FRONT_AND_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
        default:                return VK_CULL_MODE_BACK_BIT;
    }
}

static VkFrontFace front_face_to_vk(FrontFace f) {
    switch (f) {
        case FF_COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case FF_CLOCKWISE:         return VK_FRONT_FACE_CLOCKWISE;
        default:                   return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

static VkCompareOp compare_op_to_vk(CompareOp op) {
    switch (op) {
        case CO_NEVER:            return VK_COMPARE_OP_NEVER;
        case CO_LESS:             return VK_COMPARE_OP_LESS;
        case CO_EQUAL:            return VK_COMPARE_OP_EQUAL;
        case CO_LESS_OR_EQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CO_GREATER:          return VK_COMPARE_OP_GREATER;
        case CO_NOT_EQUAL:        return VK_COMPARE_OP_NOT_EQUAL;
        case CO_GREATER_OR_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CO_ALWAYS:           return VK_COMPARE_OP_ALWAYS;
        default:                  return VK_COMPARE_OP_LESS;
    }
}

static VkSampleCountFlagBits sample_count_to_vk(SampleCount sc) {
    switch (sc) {
        case SC_COUNT_1: return VK_SAMPLE_COUNT_1_BIT;
        case SC_COUNT_2: return VK_SAMPLE_COUNT_2_BIT;
        case SC_COUNT_4: return VK_SAMPLE_COUNT_4_BIT;
        case SC_COUNT_8: return VK_SAMPLE_COUNT_8_BIT;
        default:         return VK_SAMPLE_COUNT_1_BIT;
    }
}

static VkBlendFactor blend_factor_to_vk(BlendFactor f) {
    switch (f) {
        case BF_ZERO:                      return VK_BLEND_FACTOR_ZERO;
        case BF_ONE:                       return VK_BLEND_FACTOR_ONE;
        case BF_SRC_ALPHA:                 return VK_BLEND_FACTOR_SRC_ALPHA;
        case BF_ONE_MINUS_SRC_ALPHA:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BF_DST_ALPHA:                 return VK_BLEND_FACTOR_DST_ALPHA;
        case BF_ONE_MINUS_DST_ALPHA:       return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BF_SRC_COLOR:                 return VK_BLEND_FACTOR_SRC_COLOR;
        case BF_ONE_MINUS_SRC_COLOR:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BF_DST_COLOR:                 return VK_BLEND_FACTOR_DST_COLOR;
        case BF_ONE_MINUS_DST_COLOR:       return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        default:                           return VK_BLEND_FACTOR_ONE;
    }
}

static VkBlendOp blend_op_to_vk(BlendOp op) {
    switch (op) {
        case BO_ADD:               return VK_BLEND_OP_ADD;
        case BO_SUBTRACT:          return VK_BLEND_OP_SUBTRACT;
        case BO_REVERSE_SUBTRACT:  return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BO_MIN:               return VK_BLEND_OP_MIN;
        case BO_MAX:               return VK_BLEND_OP_MAX;
        default:                   return VK_BLEND_OP_ADD;
    }
}

static VkVertexInputRate input_rate_to_vk(VertexInputRate r) {
    switch (r) {
        case VIR_VERTEX:   return VK_VERTEX_INPUT_RATE_VERTEX;
        case VIR_INSTANCE: return VK_VERTEX_INPUT_RATE_INSTANCE;
        default:           return VK_VERTEX_INPUT_RATE_VERTEX;
    }
}

static VkShaderStageFlags shader_stage_to_vk(ShaderStageType stage) {
    switch (stage) {
        case ShaderStageType::SST_VERTEX:   return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStageType::SST_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStageType::SST_COMPUTE:  return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                            return VK_SHADER_STAGE_ALL_GRAPHICS;
    }
}

static VkDescriptorType desc_type_to_vk(DescriptorType t) {
    switch (t) {
        case DescriptorType::DT_SAMPLER_2D:        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::DT_UNIFORM_BUFFER:    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::DT_SHADER_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::DT_INPUT_ATTACHMENT:  return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:                                   return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::shared_ptr<zr::RenderContext> context,
                                               const GraphicsPipelineCreateInfo& info)
    : GraphicsPipeline(info), _context(context) {
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    auto device = _context->get_device();
    if (_vk_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->get_device(), _vk_pipeline, nullptr);
    }
    if (_vk_pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->get_device(), _vk_pipeline_layout, nullptr);
    }
    if (_vk_desc_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device->get_device(), _vk_desc_set_layout, nullptr);
    }
}

VkPipeline VulkanGraphicsPipeline::get_or_create(VkRenderPass render_pass, uint32_t subpass) {
    if (_vk_pipeline != VK_NULL_HANDLE && _cached_render_pass == render_pass && _cached_subpass == subpass) {
        return _vk_pipeline;
    }

    if (_vk_pipeline != VK_NULL_HANDLE) {
        auto device = _context->get_device();
        vkDestroyPipeline(device->get_device(), _vk_pipeline, nullptr);
        _vk_pipeline = VK_NULL_HANDLE;
    }

    _cached_render_pass = render_pass;
    _cached_subpass = subpass;

    auto device = _context->get_device();
    const auto& info = _ci;

    if (info.descriptor_set_layout && _vk_desc_set_layout == VK_NULL_HANDLE) {
        auto vk_layout = std::static_pointer_cast<VulkanDescriptorSetLayout>(info.descriptor_set_layout);
        const auto& ci = vk_layout->get_ci();

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (const auto& b : ci.binding_infos) {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = b.binding_idx;
            binding.descriptorCount = b.desc_count;
            binding.descriptorType = desc_type_to_vk(b.desc_type);
            binding.stageFlags = shader_stage_to_vk(b.shader_stage);
            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_ci.pBindings = bindings.data();

        CALL_VK(vkCreateDescriptorSetLayout(device->get_device(), &layout_ci, nullptr, &_vk_desc_set_layout));
    }

    if (_vk_pipeline_layout == VK_NULL_HANDLE) {
        VkPipelineLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_ci.setLayoutCount = (_vk_desc_set_layout != VK_NULL_HANDLE) ? 1 : 0;
        layout_ci.pSetLayouts = (_vk_desc_set_layout != VK_NULL_HANDLE) ? &_vk_desc_set_layout : nullptr;
        layout_ci.pushConstantRangeCount = 0;
        layout_ci.pPushConstantRanges = nullptr;

        CALL_VK(vkCreatePipelineLayout(device->get_device(), &layout_ci, nullptr, &_vk_pipeline_layout));
    }

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    auto add_shader_stage = [&](std::shared_ptr<ShaderModule> shader, VkShaderStageFlagBits stage) {
        if (!shader) return;
        auto vk_shader = std::static_pointer_cast<VulkanShaderModule>(shader);
        VkPipelineShaderStageCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stage = stage;
        ci.module = vk_shader->get();
        ci.pName = "main";
        shader_stages.push_back(ci);
    };
    add_shader_stage(info.vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
    add_shader_stage(info.fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkVertexInputBindingDescription> vk_bindings;
    std::vector<VkVertexInputAttributeDescription> vk_attrs;
    for (const auto& b : info.vertex_bindings) {
        VkVertexInputBindingDescription d{};
        d.binding = b.binding;
        d.stride = b.stride;
        d.inputRate = input_rate_to_vk(b.input_rate);
        vk_bindings.push_back(d);
    }
    for (const auto& a : info.vertex_attributes) {
        VkVertexInputAttributeDescription d{};
        d.location = a.location;
        d.binding = a.binding;
        d.format = color_format_to_vk(a.format);
        d.offset = a.offset;
        vk_attrs.push_back(d);
    }
    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(vk_bindings.size());
    vertex_input.pVertexBindingDescriptions = vk_bindings.data();
    vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(vk_attrs.size());
    vertex_input.pVertexAttributeDescriptions = vk_attrs.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = topology_to_vk(info.topology);
    input_assembly.primitiveRestartEnable = info.primitive_restart_enable ? VK_TRUE : VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = nullptr;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = nullptr;

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    VkPipelineRasterizationStateCreateInfo raster{};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.depthClampEnable = info.depth_clamp_enable ? VK_TRUE : VK_FALSE;
    raster.rasterizerDiscardEnable = VK_FALSE;
    raster.polygonMode = polygon_mode_to_vk(info.polygon_mode);
    raster.cullMode = cull_mode_to_vk(info.cull_mode);
    raster.frontFace = front_face_to_vk(info.front_face);
    raster.depthBiasEnable = VK_FALSE;
    raster.lineWidth = info.line_width;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = sample_count_to_vk(info.sample_count);
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 0.0f;
    multisample.pSampleMask = nullptr;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = info.depth_test_enable ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = info.depth_write_enable ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp = compare_op_to_vk(info.depth_compare_op);
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachments;
    for (const auto& a : info.color_blend_attachments) {
        VkPipelineColorBlendAttachmentState s{};
        s.blendEnable = a.blend_enable ? VK_TRUE : VK_FALSE;
        s.srcColorBlendFactor = blend_factor_to_vk(a.src_color_factor);
        s.dstColorBlendFactor = blend_factor_to_vk(a.dst_color_factor);
        s.colorBlendOp = blend_op_to_vk(a.color_blend_op);
        s.srcAlphaBlendFactor = blend_factor_to_vk(a.src_alpha_factor);
        s.dstAlphaBlendFactor = blend_factor_to_vk(a.dst_alpha_factor);
        s.alphaBlendOp = blend_op_to_vk(a.alpha_blend_op);
        s.colorWriteMask = a.color_write_mask;
        vk_blend_attachments.push_back(s);
    }

    VkPipelineColorBlendStateCreateInfo color_blend{};
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.attachmentCount = static_cast<uint32_t>(vk_blend_attachments.size());
    color_blend.pAttachments = vk_blend_attachments.data();

    VkGraphicsPipelineCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.stageCount = static_cast<uint32_t>(shader_stages.size());
    ci.pStages = shader_stages.data();
    ci.pVertexInputState = &vertex_input;
    ci.pInputAssemblyState = &input_assembly;
    ci.pViewportState = &viewport_state;
    ci.pRasterizationState = &raster;
    ci.pMultisampleState = &multisample;
    ci.pDepthStencilState = &depth_stencil;
    ci.pColorBlendState = &color_blend;
    ci.pDynamicState = &dynamic_state;
    ci.layout = _vk_pipeline_layout;
    ci.renderPass = render_pass;
    ci.subpass = subpass;
    ci.basePipelineHandle = VK_NULL_HANDLE;
    ci.basePipelineIndex = 0;

    CALL_VK(vkCreateGraphicsPipelines(device->get_device(), VK_NULL_HANDLE, 1, &ci, nullptr, &_vk_pipeline));

    return _vk_pipeline;
}

} // namespace vulkan
} // namespace rhi
