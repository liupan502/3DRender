//
// Created by zhida.ji1 on 2022/9/6.
//

#include "apa_application.h"
#include <common/texture_manager.h>
#include <common/model_manager.h>
#include <scenegraph/geometry/gltf_node.h>
#include <json/json.h>
using namespace hmi_app;

void ApaApplication::update(const std::string &json_content) {
    Json::Reader json_reader;
    Json::Value root;
    if (!json_reader.parse(json_content, root)) {
        Application::update(json_content);
        return;
    }

    Json::Value list = root.get("list", Json::Value::null);
    if (list == Json::Value::null) {
        return;
    }
    dirty_all_elements();

    float camera_x = 0.0f;
    float camera_y = 0.0f;
    for (int i = 0; i < list.size(); i++) {
        Json::Value val = list.get(i, Json::Value::null);
        ParkingSpaceInfo info(val);
        camera_x += info.c_x;
        camera_y += info.c_y;
        if (_parking_spaces.find(info.id) != _parking_spaces.end()) {
            _parking_spaces[info.id]->set_is_dirty(false);
            _parking_spaces[info.id]->update(info);
        }
        else {
            std::shared_ptr<ParkingSpace> parking_space = std::make_shared<ParkingSpace>(info);
            _parking_spaces.insert(std::make_pair(info.id, parking_space));
            _scene->add_node(parking_space);
        }
        // break;
    }
    camera_x /= list.size();
    camera_y /= list.size();
    std::map<int ,std::shared_ptr<ParkingSpace>>::iterator it = _parking_spaces.begin();
    for (; it != _parking_spaces.end();) {
        if (it->second->is_dirty()) {
            _scene->remove_node(it->second);
            it = _parking_spaces.erase(it);
        }
        else {
            it++;
        }
    }

    /*_scene->get_active_camera()->look_at(glm::vec3(camera_x, camera_y, 20.0f),
                                         glm::vec3(camera_x, camera_y, 0.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
    */
    Application::update(json_content);
}

void ApaApplication::update_geo(const std::string &json_content) {
    Json::Reader json_reader;
    Json::Value root;
    // const std::string tmp = "{\"list\":[{\"geo\":[[-432.41, -179.01], [-432.41, -179.06], [-432.41, -179.11], [-432.41, -179.16], [-432.41, -179.21], [-432.41, -179.26], [-432.41, -179.31], [-432.41, -179.36], [-432.41, -179.41], [-432.41, -179.46], [-432.41, -179.51], [-432.41, -179.56], [-432.41, -179.61], [-432.41, -179.66], [-432.41, -179.71], [-432.41, -179.76], [-432.41, -179.81], [-432.41, -179.86], [-432.41, -179.9], [-432.41, -179.94], [-432.41, -179.99], [-432.4, -180.02], [-432.4, -180.06], [-432.4, -180.1], [-432.4, -180.14], [-432.4, -180.17], [-432.4, -180.2], [-432.4, -180.23], [-432.4, -180.26], [-432.4, -180.29], [-432.38, -180.31], [-432.37, -180.34], [-432.35, -180.36], [-432.34, -180.38], [-432.35, -180.4], [-432.36, -180.42], [-432.37, -180.44], [-432.38, -180.45], [-432.39, -180.46], [-432.39, -180.47], [-432.41, -180.49], [-432.42, -180.49], [-432.43, -180.5], [-432.43, -180.51], [-432.44, -180.51], [-432.44, -180.51], [-432.44, -180.51], [-432.49, -180.54], [-432.53, -180.58]]}],\"ts\":1663138930033}";
    if (!json_reader.parse(json_content, root)) {
        return;
    }

    Json::Value list = root.get("list", Json::Value::null);
    if (list == Json::Value::null) {
        return;
    }

    Json::Value pts = list[0]["geo"];
    std::vector<float> buf;
    float pre_x = 0.0f;
    float pre_y = 0.0f;
    for (uint32_t i = 0; i < pts.size(); i++) {
        float x = pts[i][0].asFloat();
        float y = pts[i][1].asFloat();

        // filter same pos
        if (i > 0) {
            float len = sqrt((x - pre_x) * (x - pre_x) * (y - pre_y) * (y - pre_y));
            if (len < 0.05) {
                continue;
            }
        }
        buf.push_back(x);
        buf.push_back(y);
        pre_x = x;
        pre_y = y;
    }
    if (_geo) {
        _scene->remove_node(_geo);
    }

    _geo = std::make_shared<sg::LinePathNode>("geo", buf, 0.5f,
                                              TextureManager::get_instance()->get_tex(TEXTURE_NAME_TEST_GREEN));
    _geo->translate(0.0f, 0.0f, 0.05f);
    _scene->add_node(_geo);
    Application::update(json_content);

}

void ApaApplication::dirty_all_elements() {
    for (auto parking_space : _parking_spaces){
        parking_space.second->set_is_dirty(true);
    }
}

bool ApaApplication::init(AAssetManager* asset_mgr, ANativeWindow* window) {
    if (!Application::init(asset_mgr, window)) {
        return false;
    }



    auto camera = std::dynamic_pointer_cast<sg::PerspectiveCamera>(_scene->get_active_camera());
    /*camera->look_at(glm::vec3(-432.0, -180.0, 20.0),
                    glm::vec3(-432.0, -180.0, 0.0),
                    glm::vec3(0.0, 1.0, 0.0));*/
    camera->perspective(glm::radians(60.0f), 1.0f, 0.1f, 500.0f);


    init_self_car();

    return true;
}

std::shared_ptr<sg::Node> create_node() {
    auto node = std::make_shared<sg::Node>("");
    auto child_node1 = std::make_shared<sg::RectNode>("node1", 2, 2,
                                                      TextureManager::get_instance()->get_tex(TEXTURE_NAME_TEST_GREEN));
    // child_node1->translate(0, 1, 0.1);
    child_node1->get_component<sg::Material>()->set_depth_enabled(false);
    node->add_child(child_node1);
    child_node1->set_is_transparent(false);

    auto child_node2 = std::make_shared<sg::RectNode>("node2", 2, 2,
                                                      TextureManager::get_instance()->get_tex(hmi_app::TEXTURE_NAME_RADAR_WAVE));
    child_node2->translate(0, 0, 3.0);
    // node->add_child(child_node2);
    // child_node2->set_is_transparent(true);

    auto child_node3 = std::make_shared<sg::RectNode>("node3", 2, 2,
                                                      TextureManager::get_instance()->get_tex(hmi_app::TEXTURE_NAME_TEST_GREEN));
    child_node3->translate(0, -1, 2.9);
    // node->add_child(child_node3);
    // child_node1->set_is_transparent(true);
    return node;
}

void ApaApplication::init_self_car() {

    std::shared_ptr<sg::GltfNode> gltf_node = std::make_shared<sg::GltfNode>("models/Duck/glTF/Duck.gltf");
    gltf_node->get_component<sg::Transform>()->set_scale(glm::vec3(0.1f, 0.1f, 0.01f));
    _scene->add_node(gltf_node);

    std::shared_ptr<sg::ObjNode> obj_node = std::dynamic_pointer_cast<sg::ObjNode>(
            ModelManager::get_instance()->get_model(MODEL_NAME_OTHER_CAR));
     _self_car = std::make_shared<Car>("self_car", obj_node);
     // _self_car->children()[0]->get_component<sg::Material>()->set_specular_enabled(true);
     // _self_car->children()[0]->get_component<sg::Material>()->set_specular_exponent(10.0f);
     // _self_car->set_is_visible(false);

     std::shared_ptr<sg::RectNode> rect_node = std::make_shared<sg::RectNode>(
             "radar_wave_node", 2.4, 1.08,
             TextureManager::get_instance()->get_tex(TEXTURE_NAME_RADAR_WAVE));
     // rect_node->translate(0.0, 1.8, 0.5);
     // rect_node->rotate(1.0f, 0.0f, 0.0f, glm::radians(90.0f));
     // _self_car->add_child(rect_node);

    // _scene->add_node(_self_car);

    // _scene->add_node(rect_node);

    auto node = create_node();
    _scene->add_node(node);
    _scene->get_active_camera()->look_at(glm::vec3(0.0f, 0.0f, 15.0f),
                                         glm::vec3(0.0f, 0.0f, 0.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
}

void ApaApplication::map_pt_from_space_to_screen(float space_pt[3], float screen_pt[2]) {
    glm::vec4 pt(space_pt[0], space_pt[1], space_pt[2], 1.0f);
    glm::mat4 view_mat = _scene->get_active_camera()->get_view();
    glm::mat4 proj_mat = _scene->get_active_camera()->get_projection();
    glm::vec4 proj_vec = proj_mat * view_mat * pt;
    screen_pt[0] = (proj_vec.x / proj_vec.w + 1.0f) * 0.5;
    screen_pt[1] = (proj_vec.y / proj_vec.w + 1.0f) * 0.5;
}

void ApaApplication::update_self_car(const std::string &json_content) {
    Json::Reader json_reader;
    Json::Value root;
    if (!json_reader.parse(json_content, root)) {
        return;
    }

    float x = root["x"].asFloat();
    float y = root["y"].asFloat();
    float yaw = root["yaw"].asFloat();

    _self_car->set_is_visible(true);

    _self_car->translate(x, y, 0.0f);

    glm::quat quat1 = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat quat2 = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat quat3 = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat quat4 = glm::angleAxis(yaw, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat quat5 = quat4 * quat3 * quat2 * quat1;

    float half_theta = acos(quat5.w);
    float sin_half_theta = sin(half_theta);
    glm::vec3 vec = glm::vec3(quat5.x, quat5.y, quat5.z) / sin_half_theta;
    _self_car->rotate(vec.x, vec.y, vec.z, half_theta * 2.0f);

    glm::vec4 camer_pos(-3.0f, 0.0f, 10.0f, 1.0f);
    sg::Transform transform(nullptr);
    transform.set_translation(glm::vec3(x, y, 0.0f));
    transform.set_rotation(glm::angleAxis(yaw, glm::vec3(0.0f, 0.0f, 1.0f)));
    glm::mat4 mat = transform.get_matrix();
    glm::vec4 new_camera_pos = mat * camer_pos;

    _scene->get_active_camera()->look_at(glm::vec3(new_camera_pos.x, new_camera_pos.y, new_camera_pos.z),
                                         glm::vec3(x, y, 0.0f),
                                         glm::vec3(0.0f, 0.0f, 1.0f));

    Application::update(json_content);

    // _self_car->rotate(quat5.x, quat5.y, quat5.z, quat5.w);
}

void ApaApplication::on_clicked(float x, float y) {
    glm::vec3 click_pos = click_at_plane(x, y, 0, 0, 1, 0);
    if (click_pos.x == NAN || click_pos.y == NAN || click_pos.z == NAN) {
        return;
    }

    static float degree = 0;
    degree += 10;
    // for st
    _self_car->translate(click_pos.x, click_pos.y, click_pos.z);
    // _self_car->rotate(0.0, 1.0, 0.0, glm::radians(degree));
    update("");
}
