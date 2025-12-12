//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once
#include <memory>
#include <scenegraph/component.h>
#include <glm/common.hpp>
#include <glm/gtx/quaternion.hpp>
// #include "../../third_party/glm/common.hpp"
// #include "../../third_party/glm/gtx/quaternion.hpp"
namespace zr{
    namespace sg{
        class Node;
        class Transform : public Component{
        public:
            // Transform();
            Transform(Node* node);
            Transform(const Transform& other);
            virtual std::type_index get_type() const override;

            // inline Node* node() { return _node; };
            // inline void set_node(Node* node) { _node = node;};

        public:
            // world matrix
            glm::mat4 get_world_matrix();

            // local matrix
            glm::mat4 get_matrix();

            void set_translation(const glm::vec3& translation);
            void set_rotation(const glm::quat& rotation);
            void set_scale(const glm::vec3& scale);

            void set_matrix(const glm::mat4& mat);

            inline const glm::vec3& get_translation() const { return _translation; };
            inline const glm::quat& get_rotation() const { return _rotation; };
            inline const glm::vec3& get_scale() const { return _scale; };
            void invalidate_world_matrix();

        private:
            void reset();
            void update_world_transform();

        private:
            // Node* _node{nullptr};

            glm::vec3 _translation;
            glm::quat _rotation;
            glm::vec3 _scale;

            glm::mat4 _world_matrix;
            bool _update_world_matrix;

        };
    }
}

