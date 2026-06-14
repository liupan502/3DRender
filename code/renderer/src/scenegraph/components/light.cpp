//
// Created by root on 2022/8/8.
//

#include <scenegraph/components/light.h>
#include <scenegraph/components/texture.h>
#include <core/buffer.h>

using namespace zr::sg;

std::type_index Light::get_type() const {
    return std::type_index(typeid(Light));
}

void EnvironmentLight::set_params(const std::vector<float> &sh_params,
                                  const std::vector<std::string> &pre_filtered_img_paths,
                                  const std::string &dfg_img_path) {
    _prefiltered_tex = std::make_shared<SingleLayerTexture>(pre_filtered_img_paths, static_cast<rhi::TextureType>(TEXTURE_SAMPLER_CUBE));
    _dfg_tex = std::make_shared<SingleLayerTexture>(dfg_img_path);
    assert(sh_params.size() == 27);

    constexpr float M_SQRT_PI = 1.7724538509f;
    constexpr float M_SQRT_3  = 1.7320508076f;
    constexpr float M_SQRT_5  = 2.2360679775f;
    constexpr float M_SQRT_15 = 3.8729833462f;
    constexpr float A[9] = {
                  1.0f / (2.0f * M_SQRT_PI),    // 0  0
            -M_SQRT_3  / (2.0f * M_SQRT_PI),    // 1 -1
             M_SQRT_3  / (2.0f * M_SQRT_PI),    // 1  0
            -M_SQRT_3  / (2.0f * M_SQRT_PI),    // 1  1
             M_SQRT_15 / (2.0f * M_SQRT_PI),    // 2 -2
            -M_SQRT_15 / (2.0f * M_SQRT_PI),    // 3 -1
             M_SQRT_5  / (4.0f * M_SQRT_PI),    // 3  0
            -M_SQRT_15 / (2.0f * M_SQRT_PI),    // 3  1
             M_SQRT_15 / (4.0f * M_SQRT_PI)     // 3  2
    };

    for (uint8_t i = 0; i < 9; i++) {
        glm::vec4 vec(sh_params[i * 3], sh_params[i * 3 + 1], sh_params[i * 3 + 2], 0.0f);
        vec = vec * A[i];
        _sh_params.emplace_back(vec);
    }
}
