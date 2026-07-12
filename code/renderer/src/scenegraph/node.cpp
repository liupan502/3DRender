//
// Created by zhida.ji1 on 2022/8/8.
//

#include <scenegraph/node.h>
#include <scenegraph/scene.h>
#include <scenegraph/components/material.h>
#include <scenegraph/components/mesh.h>
#include <scenegraph/component.h>
#include <scenegraph/components/skin.h>
#include <core/device.h>
#include <core/descriptor.h>
#include <core/buffer.h>
#include <core/pipeline.h>
#include <algorithm>

using namespace zr::sg;

uint32_t Node::current_node_id = 0;

uint32_t Node::create_node_id() {
    return ++current_node_id;
}

Node::~Node() {
    _children.clear();
    _desc_sets.clear();
    _ubo_buf = nullptr;
}

Node::Node(const std::string &name) : _name (name), _is_joint(false){
    _id = Node::create_node_id();
    add_component<Transform>();
}

void Node::copy_node_children(Node* dst_node, const Node* src_node) {
    if (!dst_node || !src_node) {
        return;
    }
    for (uint16_t i = 0; i < src_node->children().size(); i++) {
        auto new_node = std::make_shared<sg::Node>(src_node->children()[i]->_name);
        dst_node->_children.emplace_back(new_node);
        new_node->set_parent(dst_node);
        Node::copy_node_children(new_node.get(), src_node->children()[i].get());
    }
}

void Node::copy_node_properties(Node *dst_node, const Node *src_node) {
    if (!dst_node || !src_node) {
        return;
    }

    dst_node->_is_visible = src_node->_is_visible;
    dst_node->_is_joint = src_node->_is_joint;
    // dst_node->_is_transparent = src_node->_is_transparent;

    dst_node->_desc_sets = src_node->_desc_sets;
    dst_node->_ubo_buf = nullptr;

    for (auto pair : src_node->_components) {
        if (!pair.second) {
            continue;
        }
        auto new_pair = std::make_pair(pair.first, pair.second);
        if (pair.first == std::type_index(typeid(sg::AnimationManager))) {
            auto mgr = std::dynamic_pointer_cast<sg::AnimationManager>(pair.second);
            auto new_mgr = std::make_shared<sg::AnimationManager>(mgr, dst_node);
            new_pair.second = new_mgr;
        }

        if (pair.first == std::type_index(typeid(sg::Transform))) {
            auto origin_transform = std::dynamic_pointer_cast<sg::Transform>(pair.second);
            auto transform = std::make_shared<sg::Transform>(*(origin_transform));
            transform->set_node(dst_node);
            new_pair.second = transform;
            dst_node->_components[new_pair.first]=new_pair.second;
            continue;
        }

        if (pair.first == std::type_index(typeid(sg::Skin))) {
            auto origin_skin = std::dynamic_pointer_cast<sg::Skin>(pair.second);
            auto skin = std::make_shared<sg::Skin>(*(origin_skin));
            skin->set_node(dst_node);
            new_pair.second = skin;
        }


        dst_node->_components.insert(new_pair);
    }

    for (uint16_t i = 0; i < src_node->children().size(); i++) {
        Node::copy_node_properties(dst_node->children()[i].get(), src_node->children()[i].get());
    }
}

void Node::adapt_skin(const Node *node) {
    for (uint16_t i = 0; i < children().size(); i++) {
        children()[i]->adapt_skin(node);
    }
    auto skin = get_component<sg::Skin>();
    if (!skin) {
        return;
    }
    std::vector<std::shared_ptr<Node>> new_joint_nodes;
    for (uint16_t i = 0; i < skin->_joint_nodes.size(); i++) {
        auto new_joint_node = node->find_node(skin->_joint_nodes[i]->_name);
        assert(new_joint_node);
        new_joint_nodes.emplace_back(new_joint_node);
    }
    skin->_joint_nodes = new_joint_nodes;
}

Node::Node(const Node& node) {
    _id = Node::create_node_id();

    Node::copy_node_children(this, &node);
    Node::copy_node_properties(this, &node);

    _name = node._name;
    _parent = nullptr;

    adapt_skin(this);
}

std::shared_ptr<Component> Node::get_component(std::type_index type_idx) {
    if (_components.find(type_idx) == _components.end()) {
        return nullptr;
    }
    else {
        return _components[type_idx];
    }
}

void Node::fetch_nodes(std::vector<std::shared_ptr<Node>> &nodes) {
    nodes.insert(nodes.end(), _children.begin(), _children.end());
    for (auto& child : _children) {
        child->fetch_nodes(nodes);
    }
}

bool Node::is_renderable(){
    if (get_component<StaticMesh>()) {
        return true;
    }

    if (get_component<DynamicMesh>()) {
        return true;
    }
    return false;
}

std::shared_ptr<Node> Node::add_child(const std::string &node_name) {
    std::shared_ptr<Node> sub_node = std::make_shared<Node>(node_name);
    add_child(sub_node);
    // _children.emplace_back(sub_node);
    return sub_node;
}

void Node::add_child(std::shared_ptr<Node> child) {
    _children.emplace_back(child);
    child->set_parent(this);
}


bool Node::upload_data(std::shared_ptr<core::Device> device) {
    // if (_components.find(std::type_index(typeid(Mesh))) != _components.end()) {
    //     get_component<Mesh>()->upload_data(device);
    // }



    /*if (is_renderable() && !_ubo_buf) {
        _ubo_buf = std::make_shared<core::UniformBuffer>(0,
                                                  sizeof(core::UniformBufferObject));
    }*/
    if(!is_visible()){
        return false;
    }
    for (auto pair : _components) {
        if (pair.second) {
            pair.second->upload_data(device);
        }
    }
    for (auto child_node : _children) {
        child_node->upload_data(device);
    }
    return true;
}

void Node::pre_frame() {
    for (auto child : _children) {
        child->pre_frame();
    }
}

void Node::remove_child(std::shared_ptr<Node> child) {
    auto it = std::find(_children.begin(), _children.end(), child);
    if (it != _children.end()) {
        _children.erase(it);
        child->set_parent(nullptr);
    }
}

void Node::translate(float x, float y, float z) {
    std::shared_ptr<Transform> transform = get_component<Transform>();
    transform->set_translation(glm::vec3(x, y, z));
}

void Node::rotate(float x, float y, float z, float radian) {

    std::shared_ptr<Transform> transform = get_component<Transform>();
    glm::quat tmp_quat = glm::normalize(glm::angleAxis(radian, glm::normalize(glm::vec3(x, y, z))));
    transform->set_rotation(tmp_quat);
}



bool Node::is_visible() const {
    Node* tmp_parent = parent();
    bool is_visible = _is_visible;
    if (parent()) {
        is_visible &= parent()->is_visible();
    }
    return is_visible;
}
std::shared_ptr<zr::core::DescriptorSet> Node::get_desc_set(std::shared_ptr<zr::core::Pipeline> pipeline) {
    if (!is_renderable()) {
        return nullptr;
    }

    if (_desc_sets.find(pipeline.get()) == _desc_sets.end()) {
        auto desc_set = pipeline->get_available_desc_set();
        _desc_sets.insert({pipeline.get(), desc_set});
    }

    return _desc_sets[pipeline.get()];
}

bool Node::contains_light()  {
    if (get_component<DirectionalLight>() != nullptr) {
        return true;
    }
    if (get_component<SpotLight>() != nullptr) {
        return true;
    }
    if (get_component<PointLight>() != nullptr) {
        return true;
    }

    if (get_component<EnvironmentLight>() != nullptr) {
        return true;
    }
    for (auto child : _children) {
        if (child->contains_light()) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<Mesh> Node::get_mesh() {

    if (get_component<StaticMesh>()) {
        return std::dynamic_pointer_cast<Mesh>(get_component<StaticMesh>());
    }

    if (get_component<DynamicMesh>()) {
        return std::dynamic_pointer_cast<Mesh>(get_component<DynamicMesh>());
    }

    return nullptr;
}

void Node::update() {
    for (auto component : _components) {
        if (!component.second) {
            continue;
        }
        component.second->update();
    }
    for (auto child : children()) {
        if (!child) {
            continue;
        }
        child->update();
    }
}

std::shared_ptr<Node> Node::find_node(const std::string &node_name) const {
    for (auto child : _children) {
        if (child->get_name() == node_name) {
            return child;
        }
        auto tmp = child->find_node(node_name);
        if (tmp) {
            return tmp;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Animation>> Node::get_animation(const std::string& animation_name) {
    std::vector<std::shared_ptr<Animation>> ret;
    auto animation_mgr = get_component<AnimationManager>();
    if (animation_mgr) {
        auto animation = animation_mgr->get_animation(animation_name);
        if (animation) {
            ret.emplace_back(animation);
        }
    }

    for (auto child : _children) {
        auto sub_ret = child->get_animation(animation_name);
        ret.insert(ret.end(), sub_ret.begin(), sub_ret.end());
    }

    return ret;
}

bool Node::is_transparent() {
    auto mat = get_component<Material>();
    if (!mat) {
        return false;
    }
    return mat->get_transparent_enabled();
}

void Node::set_is_transparent(bool is_transparent) {
    auto mat = get_component<Material>();
    if (!mat) {
        return ;
    }
    mat->set_transparent_enabled(is_transparent);
}

void Node::update_pre_world_mat() {
    set_pre_world_mat(get_component<Transform>()->get_world_matrix());
    for (auto child : _children) {
        child->update_pre_world_mat();
    }
}

bool Node::is_jitter() {
    return get_component<Material>()->is_jitter();
}
            
void Node::set_is_jitter(bool is_jitter) {
    get_component<Material>()->set_is_jitter(is_jitter);
} 
