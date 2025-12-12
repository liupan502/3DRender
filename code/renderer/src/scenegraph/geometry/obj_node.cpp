//
// Created by zhida.ji1 on 2022/9/5.
//

#include <scenegraph/geometry/obj_node.h>
#include <utils/obj_loader.h>
#include <scenegraph/scene_graph.h>
#include <sstream>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>
using namespace zr::sg;

ObjNode::ObjNode(const std::string &name, const char*obj_dir, const char* obj_name, const char* mtl_name) : Node(name){
    utils::ObjLoader loader;

    std::stringstream  ss;
    ss << obj_dir << obj_name;
    std::string obj_content = loader.load_content(ss.str().c_str());
    ss.str("");
    ss << obj_dir << mtl_name;
    std::string mtl_content = loader.load_content(ss.str().c_str());

    std::shared_ptr<sg::Scene> scene = std::make_shared<sg::Scene>();
    tinyobj::ObjReader reader;
    reader.ParseFromString(obj_content, mtl_content);
    auto shapes = reader.GetShapes();
    for (auto& shape : shapes) {
        std::shared_ptr<Node> node = std::make_shared<Node>(shape.name);
        add_child(node);
        loader.create_mesh(reader, shape, node);
        loader.create_material(reader, shape, node, obj_dir);
    }
}

ObjNode::ObjNode(const std::string name, std::shared_ptr<ObjNode> other_node) : Node(name) {
    auto other_children = other_node->children();
    for (std::shared_ptr<Node> child : other_children) {
        std::shared_ptr<Node> node = std::make_shared<Node>("");
        node->copy_component<zr::sg::DynamicMesh>(child);
        node->copy_component<zr::sg::Material>(child);
        add_child(node);
    }
}
