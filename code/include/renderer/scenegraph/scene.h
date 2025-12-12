//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <core/pipeline.h>
#include <scenegraph/components/light.h>
#include "light_manager.h"

namespace zr{
    namespace core{
        class Device;
        class LightInfo;
        class Buffer;
    }
    namespace sg {

        class Node;
        class Mesh;
        class Camera;
        class Scene {
        public:
            Scene();
            void pre_frame();
            void upload(std::shared_ptr<core::Device> device);
            void update_light();
            std::shared_ptr<Node> add_node(const std::string& node_name);
            void add_node(std::shared_ptr<Node> node);
            void remove_node(std::shared_ptr<Node> node);
            void set_active_camera(std::shared_ptr<Camera> camera) { _active_camera = camera;};
            std::shared_ptr<Camera> get_active_camera() { return _active_camera;};
            inline const std::string& name() { return _name;};
            inline void set_name(const std::string& name) { _name = name;};

            inline std::shared_ptr<LightManager> get_light_manager() { return _light_manager;};
            inline std::shared_ptr<Node> get_root() { return _root;};
            inline void update() { _root->update();};

            core::LightInfo get_light_info();
            std::vector<std::shared_ptr<Node>> get_nodes() ;
            std::vector<std::shared_ptr<Node>> get_renderable_nodes();



        protected:
            void fetch_nodes();
            void upload_light(std::shared_ptr<core::Device> device);
            std::vector<std::shared_ptr<Node>> get_light_nodes(LightType light_type);
            bool light_node_changed(std::vector<std::shared_ptr<Node>> pre_nodes,
                                    std::vector<std::shared_ptr<Node>> cur_nodes);
        private:
            bool _has_uploaded{false};
            bool _need_update_light{true};
            bool _fetch_nodes{true};
            std::string _name;
            std::shared_ptr<Node> _root{nullptr};
            std::vector<std::shared_ptr<Node>> _nodes;
            std::shared_ptr<Camera> _active_camera;
            std::shared_ptr<LightManager> _light_manager;
        };
    }
}

