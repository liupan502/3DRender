//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once

#include <string>
#include <vector>
#include <typeindex>
#include <unordered_map>
#include <memory>

#include <scenegraph/components/transform.h>
#include <scenegraph/components/animation/animation_manager.h>

namespace zr{
    namespace core{
        class Device;
        class DescriptorSet;
        class Pipeline;
        class UniformBuffer;
        struct UniformBufferObject {
            glm::mat4 world_view_projection_mat;
            glm::mat4 inverse_transpose_world_view_mat;
            glm::mat4 world_view_mat;
            glm::mat4 inverse_transpose_world_mat;
            glm::mat4 world_mat;
            glm::vec4 params;

            UniformBufferObject(glm::mat4 world_mat, glm::mat4 view_mat, 
                                glm::mat4 proj_mat, glm::vec4 params_in) {
                world_view_mat = view_mat * world_mat;
                inverse_transpose_world_view_mat = glm::transpose(glm::inverse(world_view_mat));
                world_view_projection_mat = proj_mat * world_view_mat;
                this->world_mat = world_mat;
                inverse_transpose_world_mat = glm::transpose(glm::inverse(world_mat));
                params = params_in;
            }
        };
    }
    namespace sg{


        class Component;
        class Transform;
        class Mesh;

        enum RenderingSide{
            RENDERING_NO_SIDE = 0,
            RENDERING_FRONT_SIDE = 1,
            RENDERING_BACK_SIDE = 2,
            RENDERING_DOUBLE_SIDE = 3
        };

        class Node{
            static uint32_t current_node_id;
            static uint32_t create_node_id();
        public:
            Node(const std::string& name) ;
            Node(const Node& node);
            virtual ~Node();
            void fetch_nodes(std::vector<std::shared_ptr<Node>>& nodes);
            std::shared_ptr<Node> add_child(const std::string& node_name);
            virtual void pre_frame();
            inline std::string get_name() const { return _name;};
            inline size_t id() const { return _id;};
            inline void set_parent(Node* parent) { _parent = parent; };
            inline Node* parent() const { return _parent; };
            void add_child(std::shared_ptr<Node> child);
            void remove_child(std::shared_ptr<Node> child);
            inline std::vector<std::shared_ptr<Node>> children() {return _children;};
            inline const std::vector<std::shared_ptr<Node>>& children() const {return _children;};

            inline void set_rendering_side(RenderingSide rs) { _rs = rs; };
            inline RenderingSide get_rendering_side() const { return _rs;};

            bool is_jitter(); 
            void set_is_jitter(bool is_jitter); 
            
            bool is_visible() const ;
            inline void set_is_visible(bool is_visible) { _is_visible = is_visible;};
            bool is_transparent();
            void set_is_transparent(bool is_transparent);
            std::shared_ptr<core::DescriptorSet> get_desc_set(std::shared_ptr<core::Pipeline> pipeline);
            inline std::shared_ptr<core::UniformBuffer> get_ubo_buffer() {return _ubo_buf;};

            bool is_renderable();
            virtual bool upload_data(std::shared_ptr<core::Device> device);
            bool update_uniform(std::shared_ptr<core::Device> device) {return true;};
            void translate(float x, float y, float z);
            void rotate(float x, float y, float z, float radian);
            void invalid_transform();
            bool contains_light();
            std::shared_ptr<Mesh> get_mesh();
            std::shared_ptr<sg::Node> find_node(const std::string& node_name) const;
            inline void set_is_joint(bool is_joint) { _is_joint = is_joint;};
            inline bool is_joint() const { return _is_joint;};
            inline const glm::mat4& get_pre_world_mat() const { return _pre_world_mat; };
            inline void set_pre_world_mat(const glm::mat4& pre_world_mat) { _pre_world_mat = pre_world_mat; };
            void update_pre_world_mat();
            
            std::vector<std::shared_ptr<Animation>> get_animation(const std::string& animation_name);

            virtual void update();

            inline float get_visibility() const {
                return _visibility;
            }

            inline void set_visibility(float visibility) {
                _visibility = visibility;
            }


        public:
            template<class T>
            inline std::shared_ptr<T> get_component() {
                return std::dynamic_pointer_cast<T>(get_component(typeid(T)));
            }

            template<class T>
            std::shared_ptr<T> add_component(){
                if (_components.find(std::type_index(typeid(T))) == _components.end() || 
                    !get_component<T>()) {
                    std::shared_ptr<Component> component = std::dynamic_pointer_cast<Component>(std::make_shared<T>(this));
                    _components.insert(std::make_pair(std::type_index(typeid(T)), component));
                }
                return get_component<T>();
            }

            template<class T>
            std::shared_ptr<T> add_component(std::shared_ptr<T> component){
                component->_node = this;
                if (_components.find(std::type_index(typeid(T))) == _components.end()) {
                        _components.insert(std::make_pair(std::type_index(typeid(T)), component));
                }
                else {
                    _components[std::type_index(typeid(T))] = component;
                }
                return get_component<T>();
            }

            template<class T>
                    inline void copy_component(std::shared_ptr<Node> node) {
                        if (_components.find(std::type_index(typeid(T))) == _components.end()) {
                            auto pair = std::make_pair(std::type_index(typeid(T)), node->get_component<T>());
                            _components.insert(pair);
                        }
                        else {
                            _components[std::type_index(typeid(T))] = node->get_component<T>();
                        }
                    }

        protected:
            static void copy_node_children(Node* dst_node, const Node* src_node);

            static void copy_node_properties(Node* dst_node, const Node* src_node);    

            void adapt_skin(const Node* node);        

        protected:
            std::shared_ptr<Component> get_component(std::type_index type_idx);

            std::unordered_map<std::type_index, std::shared_ptr<Component>> _components;
            
        private:
            std::string _name;
            size_t _id;
            RenderingSide _rs = RENDERING_FRONT_SIDE;
            Node* _parent{nullptr};
            std::vector<std::shared_ptr<Node>> _children;
            bool _is_visible{true};
            bool _is_joint;
            bool _is_jitter = true;
            float _visibility = 1.0f;
            glm::mat4 _pre_world_mat;

            std::map<core::Pipeline*, std::shared_ptr<core::DescriptorSet>> _desc_sets;
            std::shared_ptr<core::UniformBuffer> _ubo_buf;
        };
    }
}


