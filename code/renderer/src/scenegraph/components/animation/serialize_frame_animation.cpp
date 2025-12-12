//
// Created by zhida.ji1 on 2022/11/11.
//
#include <scenegraph/components/animation/serialize_frame_animation.h>

using namespace zr::sg;

SerializeFrameAnimation::SerializeFrameAnimation(uint32_t frame_num, uint32_t fps, TextureType tex_type,
                                                 const std::string &name) : Animation(name){
    _fps = fps;
    _frame_num = frame_num;
    _tex_type = tex_type;
    add_channel(create_channnel(_tex_type, _frame_num, _fps));
}

std::shared_ptr<AnimationChannel> SerializeFrameAnimation::create_channnel(TextureType tex_type, uint32_t frame_num,
                                                                           uint32_t fps) {
    std::shared_ptr<AnimationChannel> channel = std::make_shared<AnimationChannel>();
    channel->set_interpolation_type(ANIMATION_INTERPOLATION_TYPE_STEP);
    float step_val = 1.0f / fps;
    std::vector<float> input_data(frame_num);
    std::vector<float> output_data(frame_num);
    for (uint32_t i = 0; i < frame_num; i++) {
        input_data[i] = step_val * i;
        output_data[i] = i;
    }
    channel->set_input_buf((uint8_t*)input_data.data(), sizeof(float) * frame_num,
                           sizeof(float), frame_num);
    channel->set_output_buf((uint8_t*)output_data.data(), sizeof(float)* frame_num,
                            sizeof(float), frame_num);

    AnimationChannelType channel_type = ANIMATION_CHANNEL_TYPE_NONE;
    switch (tex_type) {
        case TEXTURE_TYPE_DIFFUSE:
            channel_type = ANIMATION_CHANNEL_TYPE_DIFFUSE;
            break;
        default:
            break;
    }
    channel->set_type(channel_type);
    return channel;
}
