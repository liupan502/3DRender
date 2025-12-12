//
// Created by zhida.ji1 on 2022/11/7.
//

#include <scenegraph/components/skin.h>
#include <core/device.h>
#include <core/buffer.h>
#include <glm/glm.hpp>
#include <scenegraph/node.h>

using namespace zr::sg;

Skin::Skin(const Skin& skin) : Component(skin){
    _inverse_bind_mats = skin._inverse_bind_mats;
    _joint_nodes = skin._joint_nodes;
    _palette_matrices_buf = nullptr;
}

void Skin::upload_data(std::shared_ptr<zr::core::Device> device) {
    // upload each time
    if (!_palette_matrices_buf) {
        uint32_t device_size = 2 * MAX_JOINT_NUM * sizeof(glm::mat4);
        _palette_matrices_buf = std::make_shared<core::UniformBuffer>(2, device, device_size);
    }
    std::vector<glm::mat4> mats(MAX_JOINT_NUM);
    for (uint32_t i = 0; i < _joint_nodes.size(); i++) {
        glm::mat4 world_mat = _joint_nodes[i]->get_component<sg::Transform>()->get_world_matrix();
        glm::mat4 mat = _joint_nodes[i]->get_component<sg::Transform>()->get_world_matrix() * _inverse_bind_mats[i];
        mats[i] = mat;
        if (!_has_pre_joints) {
            _pre_mats[i] = mat;
        }
    }
    
    uint32_t mats_size = sizeof(glm::mat4) * mats.size();
    _palette_matrices_buf->update((uint8_t*)mats.data(), mats_size);
    _palette_matrices_buf->update((uint8_t*)_pre_mats.data(), mats_size, mats_size);
    if (_has_pre_joints) {
        memcpy(_pre_mats.data(), mats.data(), sizeof(glm::mat4) * MAX_JOINT_NUM);
    }
    else {
        _has_pre_joints = true;
    }
}

void Skin::set_inverse_bind_mats_info(uint8_t* mats_ptr, uint32_t len) {
    uint32_t count = len / sizeof(glm::mat4);
    _inverse_bind_mats.clear();
    for (uint32_t i = 0; i < count; i++) {
        float* ptr = (float*)(mats_ptr + i * sizeof(glm::mat4));
        glm::mat4 mat = glm::mat4(
                glm::vec4(ptr[0], ptr[1], ptr[2], ptr[3]),
                glm::vec4(ptr[4], ptr[5], ptr[6], ptr[7]),
                glm::vec4(ptr[8], ptr[9], ptr[10], ptr[11]),
                glm::vec4(ptr[12], ptr[13], ptr[14], ptr[15]));
        _inverse_bind_mats.emplace_back(mat);
    }
}

