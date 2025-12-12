//
// Created by zhida.ji1 on 2022/10/31.
//

#include <string.h>
#include <scenegraph/components/animation/animation_channel.h>
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtx/quaternion.hpp>
#include <scenegraph/components/material.h>
#include <scenegraph/node.h>


using namespace zr::sg;

AnimationChannel::AnimationChannel(const AnimationChannel& channel) {
    _type = channel._type;
    _interpolation_type = channel._interpolation_type;

    _is_end = false;
    _pre_frame = 0;

    _offset = 0.0;
    _input_count = channel._input_count;
    _output_count = channel._output_count;
    _input_stride = channel._input_stride;
    _output_stride = channel._output_stride;
    _input_data = channel._input_data;
    _output_data = channel._output_data;
}

void AnimationChannel::set_input_buf(uint8_t * buf_ptr, uint32_t len,
                                     uint32_t stride, uint32_t count) {
    _input_data = std::vector<uint8_t>(len, 0);

    memcpy(_input_data.data(), buf_ptr, len);
    _input_stride = stride;
    _input_count = count;
}
void AnimationChannel::set_output_buf(uint8_t * buf_ptr, uint32_t len,
                                      uint32_t stride, uint32_t count) {
    _output_data = std::vector<uint8_t>(len, 0);
    memcpy(_output_data.data(), buf_ptr, len);
    _output_stride = stride;
    _output_count = count;
}

void AnimationChannel::reset() {
    _is_end = false;
    _pre_frame = 0;
}

float AnimationChannel::get_time(uint32_t idx) const {
    idx = idx % _input_count;
    float* ptr = (float *)(_input_data.data());
    return ptr[idx];
}

void AnimationChannel::update(uint64_t duration, bool is_looped, Node* node) {
    if (_is_end) {
        return;
    }

    float sec = duration * 1.0f / 1000.0f + _offset;
    float last_time = get_time(_input_count - 1);
    float first_time = get_time(0);
    float time_range = last_time - first_time;
    if (!is_looped && sec > time_range) {
        sec = time_range;
        _is_end = true;
    }
    else {
        if (sec != time_range)
        {
            sec = sec - floor(sec / time_range) * time_range;
        }
    }

    bool find = false;
    for (uint32_t i = 0; i < _input_count - 1; i ++) {
        float pre_val = get_time(_pre_frame + i) - first_time;
        float nxt_val = get_time(_pre_frame + i + 1) - first_time;
        if (pre_val <= sec && nxt_val >= sec) {
            _pre_frame += i;
            find = true;
            break;
        }
    }


    uint32_t  pre_frame = _pre_frame % (_input_count);

    float pre_time = get_time(pre_frame);
    float nxt_time = get_time(pre_frame + 1); 
    assert(nxt_time >= pre_time);

    float range = nxt_time - pre_time;
    float offset = sec - pre_time;
    std::vector<float> ret = interpolation(pre_frame, range, offset);
    apply(node, ret);
}

std::vector<float> AnimationChannel::get_vals(uint32_t idx) const {
    uint32_t num = _output_stride / sizeof(float);
    std::vector<float> ret(num, 0.0f);
    memcpy(ret.data(), _output_data.data() + _output_stride * idx, _output_stride);
    return ret;
}

std::vector<float> AnimationChannel::step_interpolation(uint32_t pre_idx, float range,
                                                        float offset) {
    std::vector<float> pre_vals = get_vals(pre_idx);
    return pre_vals;
}

std::vector<float> AnimationChannel::linear_interpolation(uint32_t pre_idx, float range, float offset) {
    std::vector<float> pre_vals = get_vals(pre_idx);
    std::vector<float> nxt_vals = get_vals(pre_idx + 1);
    std::vector<float> vals(pre_vals.size(), 0.0f);

    float ratio = offset / range;
    //
    if (_type == ANIMATION_CHANNEL_TYPE_ROTATION) {
        assert(pre_vals.size() == 4);
        glm::quat pre_quat(pre_vals[3], pre_vals[0], pre_vals[1], pre_vals[2]);
        pre_quat = normalize(pre_quat);
        glm::quat nxt_quat(nxt_vals[3], nxt_vals[0], nxt_vals[1], nxt_vals[2]);
        nxt_quat = normalize(nxt_quat);
        glm::quat cur_quat = glm::slerp(pre_quat, nxt_quat, ratio);
        vals[0] = cur_quat.w;
        vals[1] = cur_quat.x;
        vals[2] = cur_quat.y;
        vals[3] = cur_quat.z;
    }
    else {
        for (uint32_t i = 0; i < pre_vals.size(); i++) {
            vals[i] = pre_vals[i] * (1.0 - ratio)  + ratio* nxt_vals[i];
        }
    }
    return vals;
}



std::vector<float> AnimationChannel::interpolation(uint32_t pre_idx, float range, float offset) {
    std::vector<float> ret;
    switch (_interpolation_type) {
        case ANIMATION_INTERPOLATION_TYPE_CUBIC_SPLINE:
            break;
        case ANIMATION_INTERPOLATION_TYPE_STEP:
            ret = step_interpolation(pre_idx, range, offset);
            break;
        case ANIMATION_INTERPOLATION_TYPE_LINEAR:
            ret = linear_interpolation(pre_idx, range, offset);
            break;
        default:
            break;
    }
    return ret;
}

void AnimationChannel::apply(Node *node, const std::vector<float> &vals) {
    switch(_type) {
        case ANIMATION_CHANNEL_TYPE_SCALE:
        {
            assert(vals.size() == 3);
            node->get_component<Transform>()->set_scale(glm::vec3(vals[0], vals[1], vals[2]));
            break;
        }
        case ANIMATION_CHANNEL_TYPE_TRANSLATION:
        {
            assert(vals.size() == 3);
            node->get_component<Transform>()->set_translation(glm::vec3(vals[0], vals[1], vals[2]));
            break;
        }
        case ANIMATION_CHANNEL_TYPE_ROTATION:
        {
            assert(vals.size() == 4);
            node->get_component<Transform>()->set_rotation(glm::quat(vals[0], vals[1], vals[2], vals[3]));
            break;
        }
        case ANIMATION_CHANNEL_TYPE_WEIGHT:
        {
            break;
        }
        case ANIMATION_CHANNEL_TYPE_CUSTOM:
        {
            if(_custom_func){
                _custom_func(node,vals);
            }
            break;
        }
        case ANIMATION_CHANNEL_TYPE_DIFFUSE: {
            assert(vals.size() == 1);
            auto tex = node->get_component<Material>()->get_tex(TEXTURE_TYPE_DIFFUSE);
            auto multi_later_tex = std::dynamic_pointer_cast<MultiLayerTexture>(tex);
            if (multi_later_tex) {
                multi_later_tex->set_active_idx(vals[0]);
            }
            break;
        }
        default:
            break;

    }
}

void AnimationChannel::set_offset(uint32_t offset_frame) {
    if (_input_count == 1) {
        offset_frame = 0;
    }
    else {
        offset_frame = offset_frame % (_input_count - 1);
    }

    _offset = get_time(offset_frame) - get_time(0);
}
