//
// Created by zhida.ji1 on 2022/9/6.
//
#include "parking_space.h"
#include <common/model_manager.h>
#include <common/texture_manager.h>
#include <strstream>
using namespace zr;
using namespace hmi_app;

ParkingSpaceInfo::ParkingSpaceInfo(const Json::Value &val) {
    id = val.get("id", Json::Value::null).asInt();
    status = val.get("status", Json::Value::null).asInt();
    yaw = val.get("yaw", Json::Value::null).asFloat();
    c_x = val.get("p", Json::Value::null).get(Json::ArrayIndex(0), Json::Value::null).asFloat();
    c_y = val.get("p", Json::Value::null).get(Json::ArrayIndex(1), Json::Value::null).asFloat();
    height = val.get("s", Json::Value::null).get(Json::ArrayIndex(0), Json::Value::null).asFloat();
    // width = val.get("s", Json::Value::null).get(Json::ArrayIndex(1), Json::Value::null).asFloat();
    width = 3.3;
    // height = 2.3;
}

void ParkingSpace::update(const ParkingSpaceInfo &info) {
    translate(info.c_x, info.c_y, 0.0f);
    rotate(0.0f, 0.0f, 1.0f, info.yaw);
    std::shared_ptr<sg::Material> mat = get_component<sg::Material>();
    std::shared_ptr<sg::Texture> tex;
    switch(info.status) {
        case 0:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_NO_AVAILABLE);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            _car->set_is_visible(true);
            break;
        case 1:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_AVAILABLE);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            break;
        case 2:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_SELECTED);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            break;
        case 3:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_SELECTED);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            break;
        case 4:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_SELECTED);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            break;
        case 5:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_NO_AVAILABLE);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            break;
        case 6:
            tex = TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_NO_AVAILABLE);
            mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
            break;
        default:
            break;
    }
}

ParkingSpace::ParkingSpace(const ParkingSpaceInfo &info) :
    sg::RectNode("", info.width, info.height,
                 TextureManager::get_instance()->get_tex(TEXTURE_NAME_PARK_AVAILABLE)){
    std::stringstream ss;
    ss << "other_car_" << info.id;
    std::shared_ptr<sg::Node> node = ModelManager::get_instance()->get_model(MODEL_NAME_OTHER_CAR);
    _car = std::make_shared<Car>(ss.str(), std::dynamic_pointer_cast<sg::ObjNode>(node));
    _car->set_is_visible(false);
    glm::quat quat1 = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat quat2 = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat quat3 = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat quat4 = quat3 * quat2 * quat1;
    float half_theta = acos(quat4.w);
    float sin_half_theta = sin(half_theta);
    glm::vec3 vec = glm::vec3(quat4.x, quat4.y, quat4.z) / sin_half_theta;
    _car->rotate(vec.x, vec.y, vec.z, half_theta * 2.0f);
    _car->set_is_visible(false);
    add_child(_car);
    update(info);
}


