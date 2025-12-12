//
// Created by zhida.ji1 on 2022/9/8.
//

#include "model_manager.h"
#include <string>
#include <scenegraph/geometry/obj_node.h>

using namespace hmi_app;
using namespace zr::sg;

static ModelManager* _model_mgr = nullptr;

ModelManager::Destructor::~Destructor() {
    if (::_model_mgr != nullptr) {
        delete ::_model_mgr;
        ::_model_mgr = nullptr;
    }
}

ModelManager* ModelManager::get_instance() {
    if (::_model_mgr == nullptr) {
        ::_model_mgr = new ModelManager();
    }
    return ::_model_mgr;
}

std::shared_ptr<Node> ModelManager::get_model(ModelName type) {
    if (_model_map.find(type) == _model_map.end()) {
        add_obj_model(type);
    }
    return _model_map[type];
}

void ModelManager::add_obj_model(ModelName type) {
    std::string dir_path = "";
    std::string obj_name = "";
    std::string mtl_name = "";
    switch (type) {
        case MODEL_NAME_OTHER_CAR:
            dir_path = "models/other_car/";
            obj_name = "car.obj";
            mtl_name = "car.mtl";
            break;
        default:
            break;
    }

    std::shared_ptr<zr::sg::ObjNode> node = std::make_shared<zr::sg::ObjNode>("", dir_path.c_str(), obj_name.c_str(), mtl_name.c_str());
    _model_map.insert(std::make_pair(type, node));
}
