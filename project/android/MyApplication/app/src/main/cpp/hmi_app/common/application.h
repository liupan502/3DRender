//
// Created by zhida.ji1 on 2022/9/6.
//
#pragma once
#include <string>
#include <scenegraph/scene_graph.h>
#include <multipass_renderer.h>

namespace hmi_app{
    class Application{
    public:
        Application(const std::string& name = "") : _name(name) {};
        virtual bool init(AAssetManager* asset_mgr, ANativeWindow* window);
        virtual void update(const std::string& json_content);

    protected:
        glm::vec3 click_at_plane(float x, float y,
                                 float a, float b, float c, float d);
    protected:
        std::string _name{""};
        zr::MultiPassRenderer _renderer;
        std::shared_ptr<zr::sg::Scene> _scene;
        uint32_t _window_width{0};
        uint32_t  _window_height{0};

        glm::vec4 _default_camera_target;
        glm::vec3 _default_camera_pos;
    };
}
