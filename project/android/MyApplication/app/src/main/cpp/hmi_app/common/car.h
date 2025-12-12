//
// Created by zhida.ji1 on 2022/9/5.
//

#include <scenegraph/scene_graph.h>
#include <string>
using namespace zr;
namespace hmi_app {
class Car : public sg::ObjNode{
public:
    Car(const std::string& name, std::shared_ptr<sg::ObjNode>);
    // Car(const std::string& name, const char* dir_path, const char* obj_name, const char* mtl_name);
};
}
