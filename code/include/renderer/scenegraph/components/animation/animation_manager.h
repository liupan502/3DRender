//
// Created by zhida.ji1 on 2022/10/31.
//

#pragma once
#include <map>
#include <memory>
#include "../../component.h"
#include "animation_channel.h"
namespace zr {
    namespace sg{
        class Animation{

        public:
            enum AnimationState{
                ANIMATION_STATE_NONE,
                ANIMATION_STATE_DEFAULT,
                ANIMATION_STATE_PAUSING,
                ANIMATION_STATE_PLAYING,
                ANIMATION_STATE_MAX
            };
        public:
            Animation(std::string name) : _name(name), _state(ANIMATION_STATE_DEFAULT),
                _begin_ts(0), _pre_ts(0), _is_looped(false){};

            Animation(const Animation& animation);    
            inline void add_channel(std::shared_ptr<AnimationChannel> channel) { _channels.emplace_back(channel); };
            inline void set_is_looped(bool is_looped) { _is_looped = is_looped;};
            inline bool get_is_looped() const { return _is_looped; };
            inline AnimationState get_state() const { return _state;};
            void update(Node* node);
            void play(uint32_t start_frame = 0);
            void pause();
            void stop();

        private:
            virtual void update_when_pausing();
            virtual void update_when_playing(Node* node);
            float get_sec();
        private:
            std::vector<std::shared_ptr<AnimationChannel>> _channels;
            std::string _name;
            AnimationState _state;
            uint64_t _begin_ts;
            uint64_t  _pre_ts;
            bool _is_looped;
        };

        class AnimationManager : public Component{
        public:
            AnimationManager(Node* node, const std::string& name = "") : Component(node, name) {};
            AnimationManager(std::shared_ptr<AnimationManager> src, Node* node);
            // std::shared_ptr<Animation> add_animation(const std::string& name);
            void add_animation(const std::string& name, std::shared_ptr<Animation> animation);
            std::shared_ptr<Animation> get_animation(const std::string& name);
            virtual std::type_index get_type() const override { return std::type_index(typeid(AnimationManager)); };

            virtual void update() override;

            inline std::map<std::string, std::shared_ptr<Animation>> get_animations() const { return _animation_map; };
        private:
            std::map<std::string, std::shared_ptr<Animation>> _animation_map;
        };
    };
}