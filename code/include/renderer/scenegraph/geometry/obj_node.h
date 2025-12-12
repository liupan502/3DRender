//
// Created by zhida.ji1 on 2022/9/5.
//

#pragma once

#include <scenegraph/node.h>
#include <string>

namespace zr {
    namespace sg{
        class ObjNode : public Node {
        public:
            ObjNode(const std::string name, std::shared_ptr<ObjNode> other_node);
            ObjNode(const std::string& name,  const char*obj_dir, const char* obj_name, const char* mtl_name);
        };
    }
}


