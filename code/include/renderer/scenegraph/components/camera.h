//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once

#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <scenegraph/component.h>
namespace zr{
    namespace core{
        class Buffer;
        class Device;
    }
    namespace sg{
        class Node;
        class Camera : public Component{

        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        public:
            Camera(Node* node) : Component(node) {};
            // Camera() : Component(name) {};
            void look_at(glm::vec3 pos, glm::vec3 target, glm::vec3 up);
            virtual std::type_index get_type() const override;
            virtual ~Camera() = default;
            virtual glm::mat4 get_projection() = 0;
            glm::mat4 get_view();
            virtual void upload_data(std::shared_ptr<core::Device> device) override;
            void update_ubo(glm::mat4 model_mat);
            void set_node(std::shared_ptr<Node> node);
            // inline std::shared_ptr<core::Buffer> get_ubo_buffer() { return _buffer;};

            void set_model(glm::mat4 model);

            inline glm::vec3 get_pos() const { return _pos; };
            inline glm::vec3 get_target() const { return _target; };
            inline glm::vec3 get_up() const { return _up; };
        private:

            glm::vec3 _pos;
            glm::vec3 _target;
            glm::vec3 _up;
            // std::shared_ptr<core::Buffer> _buffer;
        };

        class PerspectiveCamera : public Camera {
        public:
            PerspectiveCamera(Node* node);

            virtual ~PerspectiveCamera() = default;

            virtual glm::mat4 get_projection() override;
            void perspective(float fov, float aspect_ratio, float near, float far);
        public:
            inline void set_aspect_ratio(float aspect_ratio) {_aspect_ratio = aspect_ratio; };
            inline float get_aspect_ratio() const { return _aspect_ratio;};

            inline void set_fov(float fov) { _fov = fov; };
            inline float get_fov() const { return _fov; };

            inline void set_near(float near) { _near = near; };
            inline float get_near() const { return _near; };

            inline void set_far(float far) { _far = far; };
            inline float get_far() const { return _far; };

        private:
            float _aspect_ratio{1.0f};
            float _fov{glm::radians(60.0f)};
            float _far{100.0f};
            float _near{0.1f};
        };

        class PhysicalCameraAttributes {


        public:
            PhysicalCameraAttributes() : _aperture(0.0), _shutter_speed(0.0), _sensitivity(0.0) {}
            inline float get_aperture() const { return _aperture; };
            inline void set_aperture(float aperture) { _aperture = aperture; };
            inline float get_shutter_speed() const { return _shutter_speed; };
            inline void set_shutter_speed(float shutter_speed) { _shutter_speed = shutter_speed;};
            inline float get_sensitivity() const { return _sensitivity; };
            inline void set_sensitivity(float sensitivity) { _sensitivity = sensitivity;};

            void set_exposure(float aperture, float shutter_speed, float sensitivity);
        private:
            // 光圈
            float _aperture;

            // 快门
            float _shutter_speed;

            // 敏感性
           float _sensitivity;
        };

        class PhysicalPerspectiveCamera : public PerspectiveCamera, public PhysicalCameraAttributes{
        public:
            PhysicalPerspectiveCamera(Node* node) : PerspectiveCamera(node) {};
        };
    }
}

