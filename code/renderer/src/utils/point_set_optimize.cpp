#include <utils/point_set_optimze.h>
#include <cassert>
#include <glm/glm.hpp>

using namespace zr::utils;

void choose_key_point(const std::vector<float>& input, int start_idx,
                      int end_idx, float delta, bool force_find, int& key_point_idx) {

    glm::vec2 start_pos(input[start_idx * 2], input[start_idx * 2 + 1]);
    glm::vec2 end_pos(input[end_idx * 2], input[end_idx * 2 + 1]);

    float dis = glm::length(start_pos - end_pos);
    float x_offset = fabs(start_pos.x - end_pos.x);
    float y_offset = fabs(start_pos.y - end_pos.y);

    float a = 0.0;
    float b = 0.0;
    float c = 0.0;
    if (x_offset < 0.001) {
        a = 1.0f;
        c = -start_pos.x;
    }
    else if (y_offset < 0.001) {
        b = 1.0f;
        c = -start_pos.y;
    }
    else {
        auto tmp = end_pos - start_pos;
        a = tmp.y / tmp.x;
        b = -1.0f;
        c = -(a * start_pos.x + b * start_pos.y);
    }

    float scale = 1.0f / sqrtf(a * a + b * b);
    float max_offset = force_find ? -0.1f : delta;
    key_point_idx = -1;
    std::vector<float> offset_arr;
    for (uint16_t i = start_idx + 1; i < (end_idx); i++) {
        float x = input[i * 2];
        float y = input[i * 2 + 1];
        float offset = scale * fabs(a * x + b * y + c);
        // offset_arr.emplace_back(offset);
        if (offset > max_offset) {
            key_point_idx = i;
            max_offset = offset;
        }
    }

}

void zr::utils::point_set_simplification(const std::vector<float>& input, std::vector<float>& output, std::vector<uint16_t>& output_idx) {
    assert(input.size() > 2);
    uint16_t point_num = input.size() / 2;
    float delta = 0.1f;

    output.clear();
    output.emplace_back(input[0]);
    output.emplace_back(input[1]);
    output_idx.emplace_back(0);

    int key_point_idx0 = -1;
    int key_point_idx1 = -1;
    int key_point_idx2 = -1;
    int key_point_idx3 = -1;
    int key_point_idx4 = -1;
    int key_point_idx5 = -1;
    int key_point_idx6 = -1;
    choose_key_point(input, 0, point_num - 1, delta, true, key_point_idx0);
    if (key_point_idx0 > 0) {
        choose_key_point(input, 0, key_point_idx0, delta, false, key_point_idx1);
        choose_key_point(input, key_point_idx0, point_num - 1, delta, false, key_point_idx2);
    }

    if (key_point_idx1 > 0) {
        choose_key_point(input, 0, key_point_idx1, delta, false, key_point_idx3);
        choose_key_point(input, key_point_idx1, key_point_idx0, delta, false, key_point_idx4);
        if (key_point_idx3 > 0) {
            output.emplace_back(input[key_point_idx3 * 2]);
            output.emplace_back(input[key_point_idx3 * 2 + 1]);
            output_idx.emplace_back(key_point_idx3);
        }
        output.emplace_back(input[key_point_idx1 * 2]);
        output.emplace_back(input[key_point_idx1 * 2 + 1]);
        output_idx.emplace_back(key_point_idx1);
        if (key_point_idx4 > 0) {
            output.emplace_back(input[key_point_idx4 * 2]);
            output.emplace_back(input[key_point_idx4 * 2 + 1]);
            output_idx.emplace_back(key_point_idx4);
        }
    }

    output.emplace_back(input[key_point_idx0 * 2]);
    output.emplace_back(input[key_point_idx0 * 2 + 1]);
    output_idx.emplace_back(key_point_idx0);

    if (key_point_idx2 > 0) {
        choose_key_point(input, key_point_idx0, key_point_idx2, delta, false, key_point_idx5);
        choose_key_point(input, key_point_idx2, point_num - 1, delta, false, key_point_idx6);
        if (key_point_idx5 > 0) {
            output.emplace_back(input[key_point_idx5 * 2]);
            output.emplace_back(input[key_point_idx5 * 2 + 1]);
            output_idx.emplace_back(key_point_idx5);
        }
        output.emplace_back(input[key_point_idx2 * 2]);
        output.emplace_back(input[key_point_idx2 * 2 + 1]);
        output_idx.emplace_back(key_point_idx2);
        if (key_point_idx6 > 0) {
            output.emplace_back(input[key_point_idx6 * 2]);
            output.emplace_back(input[key_point_idx6 * 2 + 1]);
            output_idx.emplace_back(key_point_idx6);
        }
    }

    output.emplace_back(input[input.size() - 2]);
    output.emplace_back(input[input.size() - 1]);
    output_idx.emplace_back(input.size() / 2 - 1);
}

glm::vec2 get_point(const std::vector<float>& input, int index) {
    return glm::vec2(input[index * 2], input[index * 2 + 1]);
}

void zr::utils::curve_fit(const std::vector<float>& input, std::vector<std::vector<glm::vec2>>& curve_params, std::vector<uint16_t>& key_point_indices) {
    uint16_t key_point_num = key_point_indices.size();
    if (key_point_num < 3) {
        return;
    }

    /*auto find_p1 = [&input](uint16_t idx0, uint16_t idx2)->glm::vec2 {
        assert(idx2 - idx0 > 1);
        glm::vec2 average_point(0.0, 0.0);
        for (uint16_t i = idx0 + 1; i < idx2; i++) {
            average_point = average_point + get_point(input, i);
        }
        average_point = average_point * (1.0f / (idx2 - idx0 - 1));

        float min_distance = glm::length(get_point(input, idx0 + 1) - average_point);
        uint16_t min_idx = idx0 + 1;
        for (uint16_t i = idx0 + 2; i < idx2; i++) {
            float distance = glm::length(get_point(input, i) - average_point);
            if (distance < min_distance) {
                min_distance = distance;
                min_idx = i;
            }
        }
        return get_point(input, min_idx);
    };*/
    for (uint16_t i = 0; i < key_point_num - 2; i++) {
        std::vector<glm::vec2> curve_param;

        glm::vec2 p0 = get_point(input, key_point_indices[i]);
        glm::vec2 p1 = get_point(input, key_point_indices[i + 1]);
        glm::vec2 p2 = get_point(input, key_point_indices[i + 2]);

        curve_param.emplace_back(p0);
        curve_param.emplace_back(p1);
        curve_param.emplace_back(p2);
        curve_params.emplace_back(curve_param);
    }

    if (curve_params.size() < 1) {
        return;
    }

    if (curve_params[0].size() == 2) {
        curve_params[0].emplace_back(curve_params[0][1]);
    }
    else {
        curve_params[0][0] = curve_params[0][0] * 2.0f - curve_params[0][1];
    }

    uint16_t last_idx = curve_params.size() - 1;
    if (curve_params[last_idx].size() == 2) {
        curve_params[last_idx].emplace_back(curve_params[last_idx][1]);
    }
    else {
        curve_params[last_idx][2] = curve_params[last_idx][2] * 2.0f - curve_params[last_idx][1];
    }

    for (uint16_t i = 0; i < last_idx; i++) {
        if (curve_params[i].size() == 2) {
            curve_params[i].emplace_back(curve_params[i][1]);
        }
    }
}


void zr::utils::sample_curves(const std::vector<std::vector<glm::vec2>>& curve_params, std::vector<float>& output, float min_distance) {
    uint16_t point_num = 0;
    for (uint16_t i = 0; i < curve_params.size(); i++) {

        uint16_t max_idx = 20;
        if (i == curve_params.size() - 1) {
            max_idx = 21;
        }
        for (uint16_t j = 0; j < max_idx; j++) {
            float t = j * 0.05;
            float param0 = pow(1.0 - t, 2.0f);
            float param1 = 1.0 + 2 * t - 2 * pow(t, 2.0f);
            float param2 = pow(t, 2.0f);
            glm::vec2 point = param0 * curve_params[i][0] + param1 * curve_params[i][1] + param2 * curve_params[i][2];
            point = point * 0.5f;
            if (output.size() == 0) {
                output.emplace_back(point.x);
                output.emplace_back(point.y);
                ++point_num;
            }
            auto last_point = get_point(output, point_num - 1);
            float distance = glm::length(point - last_point);
            if (distance > min_distance) {
                output.emplace_back(point.x);
                output.emplace_back(point.y);
                ++point_num;
            }
        }
    }
}

