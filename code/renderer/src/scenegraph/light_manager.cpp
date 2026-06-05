//
// Created by zhida.ji1 on 2022/9/26.
//
#include <scenegraph/light_manager.h>
#include <core/pipeline.h>
#include <core/buffer.h>

using namespace zr::sg;



zr::core::LightInfo LightManager::get_light_info() {
    core::LightInfo light_info{};
    light_info.directional_light_count = _directional_light_nodes.size();
    light_info.spot_light_count = _spot_light_nodes.size();
    light_info.point_light_count = _point_light_nodes.size();
    light_info.environment_light_count = _environment_light_nodes.size();
    return light_info;
}

void LightManager::upload() {
    if (!_directional_light_buffers.size()) {
        uint32_t directional_light_size = sizeof(DirectionalLight::DirectionalLightInfo);
        for (uint8_t i = 0; i < MAX_DIRECTIOANL_LIGHT_NUM; i++) {
            auto directional_light_buffer = std::make_shared<core::UniformBuffer>(13, directional_light_size);
            _directional_light_buffers.emplace_back(directional_light_buffer);
        }
    }

    if (!_spot_light_infos_buffer) {
        uint32_t spot_light_size = sizeof(SpotLight::SpotLightInfo);
        spot_light_size *= MAX_SPOT_LIGHT_NUM;
        _spot_light_infos_buffer = std::make_shared<core::UniformBuffer>(12,  spot_light_size);
    }

    if (!_point_light_infos_buffer) {
        uint32_t point_light_size = sizeof(PointLight::PointLightInfo);
        point_light_size *= MAX_POINT_LIGHT_NUM;
        _point_light_infos_buffer = std::make_shared<core::UniformBuffer>(11,  point_light_size);
    }

    if (!_light_vertex_info_buffer) {
        uint32_t light_vertex_info_size = MAX_POINT_LIGHT_NUM + MAX_SPOT_LIGHT_NUM + 1;
        light_vertex_info_size *= sizeof(glm::vec4);
        _light_vertex_info_buffer = std::make_shared<core::UniformBuffer>(1,  light_vertex_info_size);
    }

    if (_environment_light_nodes.size() > 0) {
        _environment_light_nodes[0]->get_component<zr::sg::EnvironmentLight>()
                ->get_dfg_tex()->upload_data();
        _environment_light_nodes[0]->get_component<zr::sg::EnvironmentLight>()
                ->get_prefiltered_tex()->upload_data();
        if (!_sh_buf) {
            uint32_t sh_buf_size = 9 * sizeof(glm::vec4);
            _sh_buf = std::make_shared<core::UniformBuffer>(17,  sh_buf_size);
        }
    }
}

void LightManager::set_light_nodes(LightType light_type,
                                   const std::vector<std::shared_ptr<Node>> light_nodes) {
    switch (light_type) {
        case LIGHT_TYPE_DIRECTIONAL:
            _directional_light_nodes = light_nodes;
            break;
        case LIGHT_TYPE_POINT:
            _point_light_nodes = light_nodes;
            break;
        case LIGHT_TYPE_SPOT:
            _spot_light_nodes = light_nodes;
            break;
        case LIGHT_TYPE_ENVIRONMENT:
            _environment_light_nodes = light_nodes;
            break;
        default:
            break;
    }
}

void LightManager::update(glm::mat4 view_mat) {
    update_directional(view_mat);
    update_point(view_mat);
    update_spot(view_mat);
    update_environment();
}

void LightManager::update_directional(glm::mat4 view_mat) {
    std::vector<DirectionalLight::DirectionalLightInfo> infos;
    uint8_t idx = 0;
    for (auto light_node : _directional_light_nodes) {
        auto directional_light_info = light_node->get_component<DirectionalLight>()->get_light_info();
        glm::mat4 world_mat = light_node->get_component<Transform>()->get_world_matrix();
        glm::vec4 tmp_vec = (/*view_mat */ world_mat * glm::vec4(directional_light_info.direction, 0.0));
        directional_light_info.direction = glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);
        // infos.emplace_back(directional_light_info);
        _directional_light_buffers[idx++]->update((const uint8_t*)(&directional_light_info), sizeof(DirectionalLight::DirectionalLightInfo), 0);
    }
}

void LightManager::update_point(glm::mat4 view_mat) {
    // _point_light_nodes = light_nodes;
    std::vector<PointLight::PointLightInfo> infos;
    for (auto light_node : _point_light_nodes) {
        auto point_light_info = light_node->get_component<PointLight>()->get_light_info();
        infos.emplace_back(point_light_info);
    }

    if (infos.size() == 0) {
        return;
    }

    _point_light_infos_buffer->update((const uint8_t*)(infos.data()), sizeof(PointLight::PointLightInfo) * infos.size(), 0);
}

void LightManager::update_spot(glm::mat4 view_mat) {
    // _spot_light_nodes = light_nodes;
    std::vector<SpotLight::SpotLightInfo> infos;
    for (auto light_node : _spot_light_nodes) {
        auto spot_light_info = light_node->get_component<SpotLight>()->get_light_info();
        infos.emplace_back(spot_light_info);
    }

    if (infos.size() == 0) {
        return;
    }

    _spot_light_infos_buffer->update((const uint8_t*)(infos.data()), sizeof(SpotLight::SpotLightInfo) * infos.size(), 0);
}

void LightManager::update_light_vertex_info(std::shared_ptr<sg::Camera> camera){
    int count = MAX_POINT_LIGHT_NUM + MAX_SPOT_LIGHT_NUM + 1;

    std::vector<glm::vec4> data(count, glm::vec4(0.0, 0.0, 0.0, 0.0));
    int index = 0;
    glm::mat4 view_mat = camera->get_view();
    for (uint8_t i = 0; i < _point_light_nodes.size(); i++) {
        glm::mat4 mat = view_mat * _point_light_nodes[i]->get_component<Transform>()->get_world_matrix();

        data[i].x = mat[3].x;
        data[i].y = mat[3].y;
        data[i].z = mat[3].z;
        ++index;
    }

    for (uint8_t i = 0; i < _spot_light_nodes.size(); i++) {
        glm::mat4 mat = view_mat * _spot_light_nodes[i]->get_component<Transform>()->get_world_matrix();
        data[i + MAX_POINT_LIGHT_NUM].x = mat[3].x;
        data[i + MAX_POINT_LIGHT_NUM].y = mat[3].y;
        data[i + MAX_POINT_LIGHT_NUM].z = mat[3].z;
        ++index;
    }

    glm::vec3 camera_pos = camera->get_pos();

    data[MAX_POINT_LIGHT_NUM + MAX_SPOT_LIGHT_NUM].x = camera_pos.x;
    data[MAX_POINT_LIGHT_NUM + MAX_SPOT_LIGHT_NUM].y = camera_pos.y;
    data[MAX_POINT_LIGHT_NUM + MAX_SPOT_LIGHT_NUM].z = camera_pos.z;

    _light_vertex_info_buffer->update((const uint8_t*) (data.data()), count * sizeof(glm::vec4), 0);
}

std::shared_ptr<Texture> LightManager::get_dfg_tex() const {
    if (_environment_light_nodes.size() == 0) {
        return nullptr;
    }
    return _environment_light_nodes[0]->get_component<EnvironmentLight>()->get_dfg_tex();
}
std::shared_ptr<Texture> LightManager::get_prefiltered_tex() const {
    if (_environment_light_nodes.size() == 0) {
        return nullptr;
    }
    return _environment_light_nodes[0]->get_component<EnvironmentLight>()->get_prefiltered_tex();
}

void LightManager::update_environment() {
    if (!_environment_light_nodes.size()) {
        return;
    }
    auto env_light = _environment_light_nodes[0]->get_component<EnvironmentLight>();
    const std::vector<glm::vec4>& sh_params = env_light->get_sh_params();
    _sh_buf->update((const uint8_t*)sh_params.data(), 9 * sizeof(glm::vec4));
}
