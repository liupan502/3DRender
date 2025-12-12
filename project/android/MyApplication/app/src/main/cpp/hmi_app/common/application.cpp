//
// Created by zhida.ji1 on 2022/9/6.
//
#include "application.h"
using namespace hmi_app;
using namespace zr;
bool Application::init(AAssetManager* asset_mgr, ANativeWindow* window) {
    if (!_renderer.init(asset_mgr, window)) {
        return false;
    }

    _window_width = ANativeWindow_getWidth(window);
    _window_height = ANativeWindow_getHeight(window);

    _renderer.set_viewport(0, 0, _window_width, _window_height);

    _default_camera_pos = glm::vec4(0.0, -5.0f, 5.0f, 1.0f);
    _default_camera_target = glm::vec4(0.0, 5.0f, 0.0f, 1.0f);

    _scene = std::make_shared<zr::sg::Scene>();
    std::shared_ptr<sg::Node> node = _scene->add_node("camera");
    std::shared_ptr<sg::PerspectiveCamera> camera = node->add_component<sg::PerspectiveCamera>();
    _scene->set_active_camera(camera);

    /*std::shared_ptr<sg::Node> node1 = _scene->add_node("direction_light_1");
    std::shared_ptr<sg::DirectionalLight> d_light1 = node1->add_component<sg::DirectionalLight>();
    d_light1->set_params(glm::vec3(0.5, 0.5, -0.5), glm::vec3(0.2, 0.2, 0.2));

    std::shared_ptr<sg::Node> node2 = _scene->add_node("direction_light_2");
    std::shared_ptr<sg::DirectionalLight> d_light2 = node2->add_component<sg::DirectionalLight>();
    d_light2->set_params(glm::vec3(-0.5, 0.5, -0.5), glm::vec3(0.8, 0.8, 0.8));

    std::shared_ptr<sg::Node> node3 = _scene->add_node("direction_light_3");
    std::shared_ptr<sg::DirectionalLight> d_light3 = node3->add_component<sg::DirectionalLight>();
    d_light3->set_params(glm::vec3(0.0, -0.5, 0.0), glm::vec3(0.8, 0.8, 0.8));
    */

    /*std::shared_ptr<sg::Node> node1 = _scene->add_node("point_light_1");
    std::shared_ptr<sg::PointLight> p_light1 = node1->add_component<sg::PointLight>();
    p_light1->set_params(0.01, glm::vec3(0.4, 0.3, 0.5));
    node1->translate(0.0f, 0.0f, 3.0f);*/

    /*std::shared_ptr<sg::Node> node1 = _scene->add_node("spot_light_1");
    std::shared_ptr<sg::SpotLight> s_light1 = node1->add_component<sg::SpotLight>();
    s_light1->set_params(cos(glm::radians(5.0f)), cos(glm::radians(1.0f)), 0.01,
                         glm::vec3(0.4, 0.3, 0.5), glm::vec3(0.0, 0.0, -1.0));
    node1->translate(0.0f, 0.0f, 3.0f);
    */
    return true;
}

void Application::update(const std::string& json_content) {
    _renderer.render_scene(_scene);
}

glm::vec3 Application::click_at_plane(float x, float y, float a, float b, float c, float d) {

    std::shared_ptr<sg::PerspectiveCamera> camera = std::dynamic_pointer_cast<sg::PerspectiveCamera>(_scene->get_active_camera());

    glm::mat4 view_mat = camera->get_view();
    glm::mat4 view_inverse_mat = glm::inverse(view_mat);

    float offset_x = x - _window_width * 0.5;
    float offset_y = _window_height * 0.5 - y;
    float fov = camera->get_fov();
    float offset_z = -1.0 * (_window_height * 0.5) / tan(fov * 0.5);
    glm::vec4 direction(offset_x, offset_y, offset_z, 0.0);
    glm::vec4 world_direction = view_inverse_mat * direction;
    glm::vec4 world_pos = view_inverse_mat * glm::vec4(0.0, 0.0, 0.0, 1.0);


    float val1 = world_direction.x * a + world_direction.y * b + world_direction.z * c;
    if (abs(val1) < 0.001) {
        return glm::vec3(NAN, NAN, NAN);;
    }
    float val2 = d - (world_pos.x * a + world_pos.y * b + world_pos.z * c);
    float distance = val2 / val1;
    glm::vec4 tmp = world_pos + world_direction * distance;
    return glm::vec3(tmp.x, tmp.y, tmp.z);
}


