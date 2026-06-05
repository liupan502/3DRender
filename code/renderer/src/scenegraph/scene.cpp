//
// Created by zhida.ji1 on 2022/8/8.
//

#include <scenegraph/scene.h>
#include <tinyobj/tiny_obj_loader.h>
#include <scenegraph/node.h>
#include <scenegraph/components/material.h>
#include <scenegraph/components/mesh.h>
#include <scenegraph/components/light.h>

using namespace  zr::sg;
void Scene::fetch_nodes() {
    /*if (!_fetch_nodes) {
        return;
    }*/

    _nodes.clear();
    _nodes.emplace_back(_root);
    _root->fetch_nodes(_nodes);
    _fetch_nodes = false;
}

std::vector<std::shared_ptr<Node>> Scene::get_nodes() {
    fetch_nodes();
    return _nodes;
}

/*void Scene::set_root_node(std::shared_ptr<Node> root) {
    _root = root;
    _fetch_nodes = true;
    fetch_nodes();
}*/

bool Scene::light_node_changed(std::vector<std::shared_ptr<Node>> pre_nodes,
                               std::vector<std::shared_ptr<Node>> cur_nodes) {
    if (pre_nodes.size() != cur_nodes.size()) {
        return true;
    }
    for (auto node : cur_nodes) {
        bool find = false;
        for (auto pre_node : cur_nodes) {
            if (node.get() == pre_node.get()) {
                find = true;
                break;
            }
        }
        if (!find) {
            return true;
        }
    }
    return false;
}

void Scene::upload_light(std::shared_ptr<core::Device> device) {
    if (_has_uploaded) {
        return;
    }
    _light_manager->set_light_nodes(LIGHT_TYPE_DIRECTIONAL, get_light_nodes(LIGHT_TYPE_DIRECTIONAL));
    _light_manager->set_light_nodes(LIGHT_TYPE_SPOT, get_light_nodes(LIGHT_TYPE_SPOT));
    _light_manager->set_light_nodes(LIGHT_TYPE_POINT, get_light_nodes(LIGHT_TYPE_POINT));
    _light_manager->set_light_nodes(LIGHT_TYPE_ENVIRONMENT, get_light_nodes(LIGHT_TYPE_ENVIRONMENT));

    _light_manager->upload();
    _has_uploaded = true;
}

void Scene::upload(std::shared_ptr<core::Device> device) {
    upload_light(device);
    _root->upload_data(device);
}

void Scene::update_light() {
    _light_manager->update_light_vertex_info(_active_camera);
    /*if (!_need_update_light) {
        return;
    }*/
    _light_manager->update(_active_camera->get_view());
    _need_update_light = false;
}

std::vector<std::shared_ptr<Node>> Scene::get_renderable_nodes() {
    std::vector<std::shared_ptr<Node>> nodes = get_nodes();
    std::vector<std::shared_ptr<Node>> renderable_nodes;
    for (uint32_t i = 0; i < nodes.size(); i++) {
        if (nodes[i]->is_renderable()) {
            renderable_nodes.emplace_back(nodes[i]);
        }
    }
    return renderable_nodes;
}

zr::core::LightInfo Scene::get_light_info() {
    if (_need_update_light) {
        _light_manager->set_light_nodes(LIGHT_TYPE_DIRECTIONAL, get_light_nodes(LIGHT_TYPE_DIRECTIONAL));
        _light_manager->set_light_nodes(LIGHT_TYPE_SPOT, get_light_nodes(LIGHT_TYPE_SPOT));
        _light_manager->set_light_nodes(LIGHT_TYPE_POINT, get_light_nodes(LIGHT_TYPE_POINT));
        _light_manager->set_light_nodes(LIGHT_TYPE_ENVIRONMENT, get_light_nodes(LIGHT_TYPE_ENVIRONMENT));
        _light_manager->update(_active_camera->get_view());
        _need_update_light = false;
    }

    return _light_manager->get_light_info();
}

std::vector<std::shared_ptr<Node>> Scene::get_light_nodes(LightType light_type) {

    std::vector<std::shared_ptr<Node>> nodes = get_nodes();
    std::vector<std::shared_ptr<Node>> light_nodes;
    for (uint32_t i = 0; i < nodes.size(); i++) {
        switch(light_type) {
            case LIGHT_TYPE_DIRECTIONAL:
            {
                if (nodes[i]->get_component<DirectionalLight>()) {
                    light_nodes.emplace_back(nodes[i]);
                }
                break;
            }

            case LIGHT_TYPE_POINT:
            {
                if (nodes[i]->get_component<PointLight>()) {
                    light_nodes.emplace_back(nodes[i]);
                }
                break;
            }

            case LIGHT_TYPE_SPOT:
            {
                if (nodes[i]->get_component<SpotLight>()) {
                    light_nodes.emplace_back(nodes[i]);
                }
                break;
            }

            case LIGHT_TYPE_ENVIRONMENT:
            {
                if (nodes[i]->get_component<EnvironmentLight>()) {
                    light_nodes.emplace_back(nodes[i]);
                }
                break;
            }

            default:
                break;

        }

    }
    return light_nodes;
}

Scene::Scene() {
    _root = std::make_shared<Node>("root");
    _light_manager = std::make_shared<LightManager>();
}

std::shared_ptr<Node> Scene::add_node(const std::string &node_name) {
    return _root->add_child(node_name);
}
void Scene::pre_frame() {
    _root->pre_frame();
}

void Scene::add_node(std::shared_ptr<Node> node) {
    if (node->contains_light()) {
        _need_update_light = true;
    }
    _root->add_child(node);
}

void Scene::remove_node(std::shared_ptr<Node> node) {
    if (node->contains_light()) {
        _need_update_light = true;
    }
    _root->remove_child(node);
}






