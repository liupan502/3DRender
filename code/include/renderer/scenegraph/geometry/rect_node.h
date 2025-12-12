//
// Created by zhida.ji1 on 2022/9/2.
//
#pragma once

#include <scenegraph/node.h>
#include <string>
namespace zr{

    namespace sg{
        class Texture;
        class RectNode : public Node{
        public:
            RectNode(const std::string& name, float width, float height, std::shared_ptr<Texture> tex);
            virtual ~RectNode() = default;

        private:
            int _width{0};
            int _height{0};

        };
    }
}

