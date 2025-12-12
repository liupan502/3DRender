//
// Created by zhida.ji1 on 2022/10/31.
//

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
namespace zr{
    namespace sg{

        class Node;

        enum AnimationChannelType{
            ANIMATION_CHANNEL_TYPE_NONE,
            ANIMATION_CHANNEL_TYPE_TRANSLATION,
            ANIMATION_CHANNEL_TYPE_ROTATION,
            ANIMATION_CHANNEL_TYPE_SCALE,
            ANIMATION_CHANNEL_TYPE_DIFFUSE,
            ANIMATION_CHANNEL_TYPE_WEIGHT,
            ANIMATION_CHANNEL_TYPE_CUSTOM,
            ANIMATION_CHANNEL_TYPE_MAX
        };

        enum AnimationInterpolationType {
            ANIMATION_INTERPOLATION_TYPE_NONE,
            ANIMATION_INTERPOLATION_TYPE_LINEAR,
            ANIMATION_INTERPOLATION_TYPE_STEP,
            ANIMATION_INTERPOLATION_TYPE_CUBIC_SPLINE,
            ANIMATION_INTERPOLATION_TYPE_MAX
        };
        class AnimationChannel{
        public:
            AnimationChannel() : _is_end(false), _pre_frame(0), _offset(0.0) {};
            AnimationChannel(const AnimationChannel& channel);
            inline void set_type(AnimationChannelType type) { _type = type;};
            inline AnimationChannelType get_type() const { return _type; };
            inline void set_interpolation_type(AnimationInterpolationType interpolation_type) {
                _interpolation_type = interpolation_type;
            };
            inline AnimationInterpolationType get_interpolation_type() const { return _interpolation_type; };
            inline bool is_end() const { return _is_end;};
            void set_offset(uint32_t offset_frame);
            void reset();
            void set_input_buf(uint8_t * buf_ptr, uint32_t len, uint32_t stride, uint32_t count);
            void set_output_buf(uint8_t * buf_ptr, uint32_t len, uint32_t stride, uint32_t count);
            inline  void set_custom_func(std::function<void(Node* node, const std::vector<float>& vals)> func){
                _custom_func=func;
            };
            void update(uint64_t duration, bool is_looped, Node* node);

        private:
            float get_time(uint32_t idx) const;
            std::vector<float> get_vals(uint32_t idx) const;

            std::vector<float> interpolation(uint32_t pre_idx, float range, float offset);
            std::vector<float> step_interpolation(uint32_t pre_idx, float range, float offset);
            std::vector<float> linear_interpolation(uint32_t pre_idx, float range, float offset);
            void apply(Node* node, const std::vector<float>& vals);

            std::function<void(Node* node, const std::vector<float>& vals)> _custom_func= nullptr;
        private:
            AnimationChannelType _type;
            AnimationInterpolationType _interpolation_type;

            bool _is_end;
            uint32_t  _pre_frame;
            float _offset;
            uint32_t _input_count;
            uint32_t _output_count;
            uint32_t _input_stride;
            uint32_t _output_stride;

            std::vector<uint8_t> _input_data;
            std::vector<uint8_t> _output_data;
        };
    }
}
