//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once
#include <scenegraph/component.h>
namespace zr{
    namespace sg{
        class Sampler : Component{
        public:
            Sampler(Node* node) : Component(node) {}
            virtual  std::type_index get_type() const override;
        };
    }
}



