//
// Created by zhida.ji1 on 2022/11/11.
//

#pragma once

#include <scenegraph/components/texture.h>
#include <scenegraph/components/animation/animation_manager.h>
namespace zr{
    namespace sg{
        class SerializeFrameAnimation : public Animation{
        public:
            SerializeFrameAnimation(uint32_t frame_num, uint32_t fps, TextureType type, const std::string& name);

        private:
        std::shared_ptr<AnimationChannel> create_channnel(TextureType tex_type, uint32_t frame_num, uint32_t fps);

        private:
            uint32_t  _frame_num;
            uint32_t _fps;
            TextureType _tex_type;
        };
    }
}
