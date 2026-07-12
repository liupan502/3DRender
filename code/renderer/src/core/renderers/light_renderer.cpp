#include <core/renderers/light_renderer.h>
#include <core/pipeline.h>
#include <core/pipeline_manager.h>
#include <core/descriptor.h>
#include <core/command_buffer.h>
#include <scenegraph/scene_graph.h>
#include <scenegraph/components/skin.h>
#include <map>

using namespace zr;
using namespace zr::core;

extern const unsigned int MAX_UNIFORM_BUFFER_OBJECT_COUNT = 1;

LightRenderer::LightRenderer() {

}

void LightRenderer::try_update_light_data(std::shared_ptr<sg::Node> node, 
                                    sg::Scene* scene, std::shared_ptr<core::DescriptorSet> desc_set) {
    std::shared_ptr<sg::Material> material = node->get_component<sg::Material>();
    if (!material->get_light_enabled()) {
        return;
    }
    core::LightInfo node_light_info = scene->get_light_info();

    auto d_buffers = scene->get_light_manager()->get_directional_light_buffers();
    auto s_buffer = scene->get_light_manager()->get_spot_light_buffer();
    auto p_buffer = scene->get_light_manager()->get_point_light_buffer();
    auto l_buffer = scene->get_light_manager()->get_light_vertex_buffer();

    if (node_light_info.directional_light_count && d_buffers.size()) {
        desc_set->update_desc_set_buffer(d_buffers, node_light_info.directional_light_count);
    }

    if (node_light_info.spot_light_count && s_buffer) {
        desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{s_buffer}, node_light_info.spot_light_count);
    }

    if (node_light_info.point_light_count && p_buffer) {
        desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{p_buffer}, node_light_info.point_light_count);
    }

    if (material->get_environment_enabled() && node_light_info.environment_light_count) {
        desc_set->update_desc_set_texture(scene->get_light_manager()->get_prefiltered_tex(), 15);
        desc_set->update_desc_set_texture(scene->get_light_manager()->get_dfg_tex(), 16);
        auto sh_buf = scene->get_light_manager()->get_sh_buf();
        // std::vector<std::shared_ptr<core::UniformBuffer>>
        if (sh_buf) {
            desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{sh_buf});
        }
    }

    auto pbr_params_buf = material->get_pbr_params_buf();
    if (pbr_params_buf) {
        desc_set->update_desc_set_buffer({pbr_params_buf});
    }

    if (l_buffer)
    {
        desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{l_buffer});
    }

}

void LightRenderer::prepare_renderpass(sg::Scene* scene, FgRenderPass* renderpass, const PassResources& res) {
    (void)res;
    try_init_uniform_buffers();                                            
}

void LightRenderer::try_init_uniform_buffers() {
    if (_uniform_buf) {
        return;
    }

    // 500 
    _uniform_buf = std::make_shared<UniformBuffer>(0, 
                                                sizeof(core::UniformBufferObject), 500);

    _velocity_uniform_buf = std::make_shared<UniformBuffer>(25,
                    sizeof(core::VelocityUniformBufferObject), 500);
}

UniformBufferObject LightRenderer::create_uniform_buffer_obj(std::shared_ptr<sg::Node> node, 
            std::shared_ptr<sg::Camera> camera) {
    
    glm::mat4 view = camera->get_view();
    glm::mat4 proj = camera->get_projection();

    if (_camera_info && node->is_jitter()) {
        view = _camera_info->view_mat;
        proj = _camera_info->proj_mat;
    }

    UniformBufferObject ubo(node->get_component<sg::Transform>()->get_world_matrix(),
                                          view, proj,
                                          glm::vec4(node->get_visibility(), 0.0, 0.0, 0.0));
    return ubo;
}

std::shared_ptr<Pipeline> LightRenderer::get_pipeline(std::shared_ptr<sg::Node> node, 
            const LightInfo& light_info) {
    std::shared_ptr<sg::Material> material = node->get_component<sg::Material>();
    core::LightInfo node_light_info = material->get_ability().enable_light ? light_info : core::LightInfo();
    auto pipeline = get_pipeline_mgr()->get_pipeline(node_light_info, material,
                                                                        node->get_mesh()->get_vtx_attrs()); 
    return pipeline;                                                                               

}

void LightRenderer::render_scene(sg::Scene* scene, 
            
            const PassResources& res) {
    core::LightInfo light_info = scene->get_light_info();
    auto tmp_nodes = scene->get_renderable_nodes();
    std::vector<std::shared_ptr<sg::Node>> opaque_nodes;
    std::vector<std::shared_ptr<sg::Node>> transparent_nodes;
    for (auto node : tmp_nodes) {
        if (node->is_transparent()) {
            transparent_nodes.emplace_back(node);
        }
        else {
            opaque_nodes.emplace_back(node);
        }
    }

    std::shared_ptr<sg::PerspectiveCamera> camera = std::dynamic_pointer_cast<sg::PerspectiveCamera>(scene->get_active_camera());

    // camera->set_aspect_ratio(_viewport.width / (float)_viewport.height);

    if (_is_first_frame) {
        _pre_proj_mat = camera->get_projection();
        _pre_view_mat = camera->get_view();
        _is_first_frame = false;
    }
    
    uint32_t active_idx = 0;

    auto render_mesh = [&](std::shared_ptr<Pipeline> pipeline, std::shared_ptr<sg::Mesh> mesh, uint16_t instance_count) {
        auto primitive = mesh->render_primitive();
        if (!primitive.vtx_buf || !primitive.idx_buf) {
            return;
        }
        rhi::rhi_instance->draw(pipeline->get_rhi_pipeline(), primitive, 0, mesh->get_indice_count(), instance_count);
    };


    auto render_nodes = [&, this](const std::vector<std::shared_ptr<sg::Node>>& input_nodes) {

        std::vector<std::vector<std::shared_ptr<sg::Node>>> node_arrs;
        std::map<std::shared_ptr<sg::Mesh>, std::map<std::shared_ptr<sg::Material>, uint16_t >> nodes_map;
        std::vector<std::vector<std::shared_ptr<sg::Node>>> buf_nodes_arr;
        for (uint32_t i = 0; i < input_nodes.size(); i++) {
            auto node = input_nodes[i];
            if (!node->is_visible()) {
                continue;
            }
            // 半透明节点不使用instance 渲染优化
            if (node->is_transparent()) {
                node_arrs.emplace_back(std::vector<std::shared_ptr<sg::Node>>({node}));
                continue;
            }
            auto mesh = node->get_mesh();
            auto mat = node->get_component<sg::Material>();
            if (nodes_map.find(mesh) == nodes_map.end()) {
                nodes_map.insert({mesh, std::map<std::shared_ptr<sg::Material>, uint16_t>()});
            }

            if (nodes_map[mesh].find(mat) == nodes_map[mesh].end()) {
                nodes_map[mesh].insert({mat, buf_nodes_arr.size()});
                buf_nodes_arr.emplace_back(std::vector<std::shared_ptr<sg::Node>>());
            }
            uint16_t idx = nodes_map[mesh][mat];
            buf_nodes_arr[idx].emplace_back(node);
        }

        for (uint16_t idx = 0; idx < buf_nodes_arr.size(); idx++) {

            const std::vector <std::shared_ptr<sg::Node>> &buf_node_arr = buf_nodes_arr[idx];
            if (buf_node_arr.size() < MAX_UNIFORM_BUFFER_OBJECT_COUNT) {
                node_arrs.emplace_back(std::move(buf_node_arr));
                continue;
            }
            uint16_t start_idx = 0;
            while (true) {
                uint16_t end_idx = start_idx + MAX_UNIFORM_BUFFER_OBJECT_COUNT;
                if (end_idx < buf_node_arr.size()) {
                    node_arrs.emplace_back(
                            std::vector < std::shared_ptr < sg::Node >> (buf_node_arr.begin() + start_idx,
                                    buf_node_arr.begin() + end_idx));
                } else {
                    node_arrs.emplace_back(
                            std::vector < std::shared_ptr < sg::Node >> (buf_node_arr.begin() + start_idx,
                                    buf_node_arr.end()));
                    break;
                }
                start_idx += MAX_UNIFORM_BUFFER_OBJECT_COUNT;
            }
        }

        for (uint16_t i = 0; i < node_arrs.size(); i++) {

            auto node = node_arrs[i][0];
            std::shared_ptr<sg::Material> material = node->get_component<sg::Material>();
            auto pipeline = get_pipeline(node, light_info);
            std::shared_ptr<core::DescriptorSet> desc_set = node->get_desc_set(pipeline);

            std::shared_ptr<sg::Texture> tex_diffuse = material->get_tex(sg::TEXTURE_TYPE_DIFFUSE);
            if (tex_diffuse) {
                desc_set->update_desc_set_texture(tex_diffuse, 10);
            }

            auto fragment_uniform_buf = material->get_fragment_uniform_buf();
            if (fragment_uniform_buf) {
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{fragment_uniform_buf});
            }

            auto skin = node->get_component<sg::Skin>();
            if (material->get_skin_enabled() && skin) {
                auto palette_matrices_buf = skin->get_palette_matrices_buf();
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{palette_matrices_buf}, 1);
            }

            try_update_light_data(node, scene, desc_set);

            std::vector<std::shared_ptr<core::UniformBuffer>> uniform_buffers;
            for (uint16_t j = 0; j < node_arrs[i].size(); j++) {

                VelocityUniformBufferObject velocity_uniform_buf_obj;
                glm::mat4 world_mat = node_arrs[i][j]->get_component<sg::Transform>()->get_world_matrix();
                velocity_uniform_buf_obj.current_world_view_proj_mat = camera->get_projection() * camera->get_view() * world_mat;
                velocity_uniform_buf_obj.pre_world_view_proj_mat = _pre_proj_mat * _pre_view_mat * node_arrs[i][j]->get_pre_world_mat();
                _velocity_uniform_buf->set_active_element(active_idx);
                _velocity_uniform_buf->update((const uint8_t*) &velocity_uniform_buf_obj, 
                            sizeof(core::VelocityUniformBufferObject), 0, false);
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{_velocity_uniform_buf}, 1, j);            

                _uniform_buf->set_active_element(active_idx++);
                auto ubo = create_uniform_buffer_obj(node_arrs[i][j], camera);
                _uniform_buf->update((const uint8_t*) &ubo, sizeof(core::UniformBufferObject), 0, false);
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{_uniform_buf}, 1, j);
                
                
            }
        }


        for (uint16_t i = 0; i < node_arrs.size(); i++) {

            auto node = node_arrs[i][0];
            auto pipeline = get_pipeline(node, light_info);
            std::shared_ptr<sg::Mesh> mesh = node->get_mesh();
            uint16_t instance_count = node_arrs[i].size();
            render_mesh(pipeline, mesh, instance_count);
        }


    };

    //vkCmdSetViewport(cmd_buf->get(), 0, 1, &_viewport);


    render_nodes(opaque_nodes);
    // render_nodes(transparent_nodes);
    for (auto node : transparent_nodes) {
        auto mat = node->get_component<sg::Material>();
        if (!mat) {
            continue;
        }
        mat->set_cull_mode(2);
    }
    render_nodes(transparent_nodes);

    std::vector<std::shared_ptr<sg::Node>> double_side_transparent_nodes;
    for (auto node : transparent_nodes) {
        auto mat = node->get_component<sg::Material>();
        if (!mat) {
            continue;
        }
        if (node->get_rendering_side() != sg::RENDERING_DOUBLE_SIDE) {
            continue;
        }
        mat->set_cull_mode(1);
        double_side_transparent_nodes.emplace_back(node);
    }
    render_nodes(double_side_transparent_nodes);
    for (auto node : transparent_nodes) {
        auto mat = node->get_component<sg::Material>();
        if (!mat) {
            continue;
        }
        
        mat->set_cull_mode(0);
    }

    _pre_proj_mat = camera->get_projection();
    _pre_view_mat = camera->get_view();

    for (auto node : tmp_nodes) {
        node->set_pre_world_mat(node->get_component<sg::Transform>()->get_world_matrix());
    }
}

CreatePipelineFunc LightRenderer::get_pipeline_creator() {
    CreatePipelineFunc cp = [](PipelineFeature feature,
                               std::shared_ptr<FgRenderPass> fg_renderpass ,
                               uint16_t subpass_idx) -> std::shared_ptr<Pipeline>{
        return std::make_shared<BasePipeline>(fg_renderpass, subpass_idx, feature);
    };

    return cp;
}

