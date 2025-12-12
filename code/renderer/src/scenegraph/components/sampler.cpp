//
// Created by root on 2022/8/8.
//

#include <scenegraph/components/sampler.h>

using namespace zr::sg;

std::type_index Sampler::get_type() const {
    return std::type_index(typeid(Sampler));
}