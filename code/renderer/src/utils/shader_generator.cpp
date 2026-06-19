//
// Created by zhida.ji1 on 2022/9/14.
//
#include <utils/shader_generator.h>
#include <core/pipeline.h>
#include <sstream>

using namespace zr::utils;

std::string ShaderGenerator::compute_variant_key(core::PipelineFeature& feature) {
    auto light_info = feature.get_light_info();
    auto ability = feature.get_material_ability();

    uint32_t d = light_info.directional_light_count;
    uint32_t p = light_info.point_light_count;
    uint32_t s = light_info.spot_light_count;
    uint32_t sk = ability.enable_skin ? 1 : 0;
    uint32_t env = (ability.enable_environment && light_info.environment_light_count > 0) ? 1 : 0;
    uint32_t vc = ability.enable_color ? 1 : 0;

    std::stringstream ss;
    ss << "d" << d << "_p" << p << "_s" << s
       << "_sk" << sk << "_env" << env << "_vc" << vc;
    return ss.str();
}

std::string ShaderGenerator::get_shader_path(const std::string& shader_name,
                                              core::PipelineFeature& feature) {
    // Compute variant key for all shaders that use defines
    // The key is appended to the filename: {name}.{ext}.{key}.spv
    std::string key = compute_variant_key(feature);
    // shader_name = "textured.vert" or "textured.frag"
    size_t dot_pos = shader_name.rfind('.');
    std::string base_name = shader_name.substr(0, dot_pos);  // "textured"
    std::string ext = shader_name.substr(dot_pos + 1);        // "vert" or "frag"
    return std::string("shaders/spv/") + base_name + "." + ext + "." + key + ".spv";
}

std::string ShaderGenerator::get_simple_shader_path(const std::string& shader_name) {
    return std::string("shaders/spv/") + shader_name + ".spv";
}
