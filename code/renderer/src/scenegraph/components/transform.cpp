//
// Created by root on 2022/8/8.
//

#include <scenegraph/components/transform.h>
#include <scenegraph/node.h>
#include <glm/gtx/matrix_decompose.hpp>

using namespace zr::sg;

std::type_index Transform::get_type() const {
    return std::type_index(typeid(Transform));
}

Transform::Transform(Node* node) : Component(node, "transform"){
    set_scale(glm::vec3(1.0f, 1.0f, 1.0f));
    set_translation(glm::vec3(0.0f, 0.0f, 0.0f));
    set_rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
}

void Transform::set_translation(const glm::vec3& translation) {
    _translation = translation;
    invalidate_world_matrix();
}

void Transform::set_scale(const glm::vec3& scale) {
    _scale = scale;
    invalidate_world_matrix();
}

void Transform::set_rotation(const glm::quat& rotation) {
    _rotation = rotation;
    invalidate_world_matrix();
}

glm::mat4 Transform::get_matrix() {
    glm::mat4 translate_mat = glm::translate(glm::mat4(1.0), get_translation());
    glm::mat4 rotation_mat = glm::mat4_cast(get_rotation());
    glm::mat4 scale_mat = glm::scale(glm::mat4(1.0), get_scale());

    glm::mat4 mat = translate_mat * rotation_mat * scale_mat;
    return mat;
}

glm::mat4 Transform::get_world_matrix() {
    update_world_transform();
    return _world_matrix;
}

void Transform::update_world_transform() {
    if (!_update_world_matrix) {
        return;
    }

    _world_matrix = get_matrix();
    auto node = get_node();
    auto parent = node->parent();

    if (node->is_joint() && !parent->is_joint()) {
        _update_world_matrix = false;
        return;
    }

    if (parent) {
        auto parent_transform = parent->get_component<Transform>();
        _world_matrix = parent_transform->get_world_matrix() * _world_matrix;
    }
    _update_world_matrix = false;
}

void Transform::invalidate_world_matrix() {
    _update_world_matrix = true;
    if (get_node()) {
        for(auto child : get_node()->children()) {
            child->get_component<Transform>()->invalidate_world_matrix();
        }
    }
}

void Transform::set_matrix(const glm::mat4 &mat) {
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 tranlation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rotation, tranlation, skew, perspective);
    set_scale(scale);
    set_translation(tranlation);
    set_rotation(rotation);
}

Transform::Transform(const Transform& other) : Component(other) {
    _translation = other._translation;
    _rotation = other._rotation;
    _scale = other._scale;
    _update_world_matrix = true;
}
