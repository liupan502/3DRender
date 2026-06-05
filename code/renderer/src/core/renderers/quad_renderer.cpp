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
    /*auto* vulkan_rhi = static_cast<rhi::vulkan::VulkanRHI*>(rhi::rhi_instance);
    std::shared_ptr<core::Device> device = vulkan_rhi->get_context()->get_device();
    
    if (!_quad_mesh_buf) {
        _quad_mesh_buf = std::make_shared<core::Buffer>(device, (VkDeviceSize)(6 * sizeof(float)), 
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        float data[6] = {-1.0f, -3.0f, -1.0f, 1.0f, 3.0f, 1.0f};
        _quad_mesh_buf->update((uint8_t*)data, sizeof(float) * 6);                    
    }

    prepare_desc(renderpass, device); */   
}

void QuadRenderer::reset_viewport(FgRenderPass *renderpass) {
    auto display_size = renderpass->get_display_size();
    _viewport = {0, 0,
                 (float)display_size.width, (float)display_size.height, 0, 1.0};
}

void QuadRenderer::prepare_desc(FgRenderPass* renderpass, std::shared_ptr<Device> device) {
    /*if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_desc_pool()->get_available_desc_sets(1)[0];

        // _desc_set->update_desc_set_texture(nullptr, 0);
    }

    auto view = renderpass->get_input_views()[0];
    // _desc_set->update_desc_set_input_attachment(view, 0);
    std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>(device);
    _desc_set->update_desc_set_texture(sampler, view, 0);

    reset_viewport(renderpass);*/
}

void QuadRenderer::render_scene(sg::Scene* scene, 
            const PassResources& res) {
    
    /*vkCmdSetViewport(_cmd_buf->get(), 0, 1, &_viewport);
    _pipeline->bind(_cmd_buf);
    _desc_set->bind(_cmd_buf, _pipeline->get_pipeline_layout());
    VkDeviceSize offset = 0;
    VkBuffer vtx_buf = _quad_mesh_buf->get();
    vkCmdBindVertexBuffers(_cmd_buf->get(), 0, 1, &vtx_buf, &offset);
    vkCmdDraw(_cmd_buf->get(), 3, 1, 0, 0);*/
}

QuadPipeline::QuadPipeline(
            std::shared_ptr<FgRenderPass> render_pass, uint32_t subpass_idx, 
            PipelineFeature feature
            , const std::map<std::string, std::string>& shader_path_map, 
                const std::vector<rhi::DescriptorBindingInfo>& binding_infos) : _shader_path_map(shader_path_map),
                Pipeline(feature) {

    /*std::vector<DescriptorBindingInfo> binding_infos;
    DescriptorBindingInfo binding_info{};
    binding_info.binding_idx = 0;
    binding_info.desc_count = 1;
    // binding_info.desc_type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    binding_info.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding_info.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding_infos.emplace_back(binding_info);*/
    
    // _desc_pool = std::make_shared<DescriptorPool>(_device, binding_infos, 10);
    // _desc_layout = _desc_pool->get_layout();
    _desc_layout = rhi::rhi_instance->create_descriptor_set_layout({ binding_infos });
    create(render_pass, subpass_idx, _desc_layout);
}

void QuadPipeline::create_color_blend_state() {
    _color_blend_state = {};
    _color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _color_blend_state.logicOpEnable = VK_FALSE;
    _vk_pipeline_color_blend_states.clear();
    VkPipelineColorBlendAttachmentState vk_pipeline_color_blend_state = VkPipelineColorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    _vk_pipeline_color_blend_states.emplace_back(vk_pipeline_color_blend_state);
    _color_blend_state.pAttachments = _vk_pipeline_color_blend_states.data();
    _color_blend_state.attachmentCount = _vk_pipeline_color_blend_states.size();
}

void QuadPipeline::create_vtx_input_state() {
    _vk_input_binding_descs.clear();
    _vk_input_attri_descs.clear();
    VkVertexInputBindingDescription binding_desc = VkVertexInputBindingDescription{
                .binding = 0,
                .stride = sizeof(float) * 2,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    _vk_input_binding_descs.emplace_back(binding_desc);

    auto attr_desc = VkVertexInputAttributeDescription{
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = 0};
    _vk_input_attri_descs.emplace_back(attr_desc);            
    

    _vtx_input_state =   VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(_vk_input_binding_descs.size()),
            .pVertexBindingDescriptions = _vk_input_binding_descs.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(_vk_input_attri_descs.size()),
            .pVertexAttributeDescriptions = _vk_input_attri_descs.data(),
    };
}

void QuadPipeline::create_depth_stencil_state() {
    _depth_stencil_state = VkPipelineDepthStencilStateCreateInfo{};
    _depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depth_stencil_state.stencilTestEnable = VK_FALSE;
    _depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    _depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
    _depth_stencil_state.depthWriteEnable = VK_FALSE;
    _depth_stencil_state.depthTestEnable = VK_FALSE;
    _depth_stencil_state.minDepthBounds = 0.0f;
    _depth_stencil_state.maxDepthBounds = 1.0f;
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
        

