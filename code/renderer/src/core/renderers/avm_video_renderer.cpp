#include <core/renderers/avm_video_renderer.h>
#include <core/fg/fg_render_pass.h>

using namespace zr::core;

void AvmVideoRenderer::prepare_desc(FgRenderPass* render_pass, std::shared_ptr<Device> device) {
    if (!_desc_set) {
        _pipeline = _pipeline_mgr->get_pipeline(LightInfo(), std::make_shared<sg::Material>(nullptr),
                                    std::vector<std::vector<sg::VertexAttribute>>());
        _desc_set = _pipeline->get_desc_pool()->get_available_desc_sets(1)[0];
    }

    if (!_config_uniform_buf) {
        _config_uniform_buf = std::make_shared<UniformBuffer>(10, device, sizeof(AvmConfigInfo));
    }

    if (!_surround_camera_intri_buf) {
        _surround_camera_intri_buf = std::make_shared<UniformBuffer>(11, device, sizeof(SurroundCameraIntrinsics), 4);
    }

    _desc_set->update_desc_set_texture(_surround_tex, 0);

    

    assert(_surround_camera_intri_arr.size() == 4);
    for (uint8_t i = 0; i < _surround_camera_intri_arr.size(); i++) {
        _surround_camera_intri_buf->set_active_element(i);
        _surround_camera_intri_buf->update((const uint8_t*)(&_surround_camera_intri_arr[i]), 
                sizeof(SurroundCameraIntrinsics));
        _desc_set->update_desc_set_buffer(
            std::vector<std::shared_ptr<UniformBuffer>>({_surround_camera_intri_buf}), 1, i);
    }
    
    auto display_size = render_pass->get_display_size();
    _config_info.frame_info.x = display_size.width;
    _config_info.frame_info.y = display_size.height;
    _config_uniform_buf->update((const uint8_t*)(&_config_info), sizeof(AvmConfigInfo));
    _desc_set->update_desc_set_buffer({_config_uniform_buf});
    reset_viewport(render_pass);
}

CreatePipelineFunc AvmVideoRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](std::shared_ptr<Device> device, PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,
                     uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        std::map<std::string, std::string> shader_path_map;
        shader_path_map.insert({"vert", "shaders/quad.vert"});  
        shader_path_map.insert({"frag", "shaders/avm_video.frag"}); 

        std::vector<DescriptorBindingInfo> binding_infos;
        DescriptorBindingInfo binding_info_0{};
        binding_info_0.binding_idx = 0;
        binding_info_0.desc_count = 1;
        binding_info_0.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_0.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_0); 

        DescriptorBindingInfo binding_info_1{};
        binding_info_1.binding_idx = 1;
        binding_info_1.desc_count = 1;
        binding_info_1.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding_info_1.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_1);  

        DescriptorBindingInfo binding_info_2{};
        binding_info_2.binding_idx = 10;
        binding_info_2.desc_count = 1;
        binding_info_2.desc_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding_info_2.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_2);

        DescriptorBindingInfo binding_info_3{};
        binding_info_3.binding_idx = 11;
        binding_info_3.desc_count = 4;
        binding_info_3.desc_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding_info_3.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding_infos.emplace_back(binding_info_3);

        return std::make_shared<QuadPipeline>(device, renderpass, subpass_idx, feature, shader_path_map, binding_infos);
    };
    return cp;                 
}

void AvmVideoRenderer::set_mode(AvmMode mode) {
    switch (mode) {
        case AVM_MODE_SOURROUND_LEFT : {
            _config_info.sampler_info.x = 0;
            _config_info.sampler_info.y = 0;
            break;
        }
        case AVM_MODE_SOURROUND_RIGHT : {
            _config_info.sampler_info.x = 0;
            _config_info.sampler_info.y = 1;
            break;
        }
        case AVM_MODE_SOURROUND_REAR : {
            _config_info.sampler_info.x = 0;
            _config_info.sampler_info.y = 2;
            break;
        }
        case AVM_MODE_SOURROUND_FRONT : {
            _config_info.sampler_info.x = 0;
            _config_info.sampler_info.y = 3;
            break;
        }
        default:
            break;
    }
}


