//
// Created by root on 2022/8/8.
//

#include <scenegraph/components/camera.h>
#include <scenegraph/node.h>
#include <scenegraph/components/transform.h>
#include <core/buffer.h>
#include <vulkan/vulkan.h>
using namespace zr::sg;

std::type_index Camera::get_type() const {
    return std::type_index(typeid(Camera));
}

void Camera::look_at(glm::vec3 pos, glm::vec3 target, glm::vec3 up) {
    _pos = pos;
    _target = target;
    _up = up;
}

glm::mat4 Camera::get_view(){
    return glm::lookAt(_pos, _target, _up);
}

glm::mat4 PerspectiveCamera::get_projection() {
    glm::mat4 proj = glm::perspective(_fov, _aspect_ratio, _near, _far);
    proj[1][1] *= -1.0f;
    return proj;
}

void Camera::set_model(glm::mat4 model) {
    UniformBufferObject ubo;
    ubo.model = model;
    ubo.proj = get_projection();
    ubo.view = get_view();

}

PerspectiveCamera::PerspectiveCamera(Node* node) : Camera(node){
    // set_name(name);
}

void PerspectiveCamera::perspective(float fov, float aspect_ratio, float near, float far) {
    _fov = fov;
    _aspect_ratio = aspect_ratio;
    _near = near;
    _far = far;
}

void Camera::upload_data(std::shared_ptr<core::Device> device) {
    if (_has_uploaded) {
        return;
    }
    // _buffer = std::make_shared<core::Buffer>(device, (VkDeviceSize)(sizeof(UniformBufferObject)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    _has_uploaded = true;
}

void Camera::update_ubo(glm::mat4 model_mat) {
    UniformBufferObject ubo;
    ubo.view = get_view();
    ubo.proj = get_projection();
    ubo.model = model_mat;
    // _buffer->update((const uint8_t *)&ubo, sizeof(ubo));
}

void PhysicalCameraAttributes::set_exposure(float aperture, float shutter_speed,
                                            float sensitivity) {
    set_aperture(aperture);
    set_shutter_speed(shutter_speed);
    set_sensitivity(sensitivity);
}
