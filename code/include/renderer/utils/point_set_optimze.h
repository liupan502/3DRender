
#pragma once
#include <vector>
#include <glm/glm.hpp>
namespace zr {
    namespace utils {
    void point_set_simplification(const std::vector<float>& input, std::vector<float>& output, std::vector<uint16_t>& output_idx);
    void curve_fit(const std::vector<float>& input, std::vector<std::vector<glm::vec2>>& curve_params, std::vector<uint16_t>& key_point_indices);
    void sample_curves(const std::vector<std::vector<glm::vec2>>& curve_params, std::vector<float>& output, float min_distance);
};
};


