//
// Created by zhida.ji1 on 2022/9/5.
//
#include "car.h"
using namespace zr;
using namespace hmi_app;

//Car::Car(const std::string &name, const char *dir_path, const char *obj_name,
//         const char *mtl_name) : sg::ObjNode(name, dir_path, obj_name, mtl_name){
//
//}

Car::Car(const std::string& name, std::shared_ptr<sg::ObjNode> obj_node) : sg::ObjNode(name, obj_node) {
    // _components.insert(std::make_pair(std::type_index(typeid(sg::Mesh)), obj_node->get_component<sg::Mesh>()));
    // _components.insert(std::make_pair(std::type_index(typeid(sg::Material)), obj_node->get_component<sg::Material>()));
}
