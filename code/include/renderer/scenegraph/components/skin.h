//
// Created by zhida.ji1 on 2022/11/7.
//
#include <scenegraph/component.h>
#include <glm/glm.hpp>
#include <vector>

#define MAX_JOINT_NUM 100

namespace zr {
    namespace core {
        class Device;
        class UniformBuffer;
    }
    namespace sg{

        class Skin : public Component{
        friend class Node;
        public:
            Skin(Node* node, const std::string& name = "") : Component(node, name), _palette_matrices_buf(nullptr){};
            Skin(const Skin& skin);
            void set_inverse_bind_mats_info(uint8_t* mats_ptr, uint32_t len);
            inline void reset_pre_joints() { _has_pre_joints = false; };
            inline void set_joint_nodes(std::vector<std::shared_ptr<sg::Node>>& joint_nodes) { _joint_nodes = joint_nodes;};
            inline std::shared_ptr<core::UniformBuffer> get_palette_matrices_buf() const { return _palette_matrices_buf;};
            virtual void upload_data(std::shared_ptr<core::Device> device) override;
            virtual std::type_index get_type() const override { return std::type_index(typeid(Skin)); };
        private:
            std::vector<glm::mat4> _inverse_bind_mats;
            std::vector<std::shared_ptr<sg::Node>> _joint_nodes;
            std::shared_ptr<core::UniformBuffer> _palette_matrices_buf;
            std::vector<glm::mat4> _pre_mats = std::vector<glm::mat4>(MAX_JOINT_NUM);
            bool _has_pre_joints = false;
        };
    }
}
