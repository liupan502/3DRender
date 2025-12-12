//
// Created by zhida.ji1 on 2022/9/2.
//
#pragma once
#include <scenegraph/node.h>
#include <string>
#include <vector>
namespace zr{
    namespace sg{
        class Texture;
        class LinePathNode : public Node{
        public:
            LinePathNode(const std::string& name, const std::vector<float> line_path_data,
                         float width, std::shared_ptr<Texture> tex, bool use_tex_len = false, float tex_len = 0.0f);
        };
    }
}
