//
// Created by zhida.ji1 on 2022/10/31.
//

#include <scenegraph/components/animation/animation_manager.h>
#ifdef WIN32
#include<windows.h>
#else
#include <sys/time.h>
#endif 

using namespace zr::sg;

Animation::Animation(const Animation& animation) {

    _name = animation._name;
    _state = AnimationState::ANIMATION_STATE_DEFAULT;
    _begin_ts = 0;
    _pre_ts = 0;
    _is_looped = false;

    for (uint16_t i = 0; i < animation._channels.size(); i++) {
        _channels.emplace_back(std::make_shared<AnimationChannel>(*(animation._channels[i])));
    }
}

AnimationManager::AnimationManager(std::shared_ptr<AnimationManager> src, Node* node) :
    Component(node, src->name()){
    auto it = src->_animation_map.begin();
    for (; it != src->_animation_map.end(); it++) {
        _animation_map.insert(std::make_pair(it->first,
                                std::make_shared<Animation>(*(it->second))));
    }
}

uint64_t get_now_ms() {
#ifdef WIN32
    FILETIME ft;
    LARGE_INTEGER li;

    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    // 转换为毫秒表示
    return li.QuadPart / 10000;
#else
    struct timeval time;
    gettimeofday(&time, nullptr);
    return uint64_t(time.tv_sec * 1000 + time.tv_usec / 1000);
#endif 
}

 void AnimationManager::add_animation(const std::string& name, std::shared_ptr<Animation> animation) {
    _animation_map.insert(std::make_pair(name, animation));
}

std::shared_ptr<Animation> AnimationManager::get_animation(const std::string &name) {
    if (_animation_map.find(name) == _animation_map.end()) {
        return nullptr;
    }
    return _animation_map[name];
}

void AnimationManager::update() {
    for(auto pair : _animation_map) {
        pair.second->update(get_node());
    }
}

void Animation::update(Node* node) {
    switch (_state) {
        case ANIMATION_STATE_PLAYING:
            update_when_playing(node);
            break;
        case ANIMATION_STATE_PAUSING:
            update_when_pausing();
            break;
        default:
            break;
    }
    _pre_ts = get_now_ms();
    return;
}

void Animation::update_when_pausing() {
    uint64_t now = get_now_ms();
    _begin_ts += (now - _pre_ts);
}

void Animation::update_when_playing(Node* node) {
    uint64_t duration = get_now_ms() - _begin_ts;
    bool has_playing_channel = false;
    for (auto channel : _channels) {
        if (channel->is_end()) {
            continue;
        }
        has_playing_channel = true;
        channel->update(duration, _is_looped, node);
    }
    if (!has_playing_channel) {
        _state = ANIMATION_STATE_DEFAULT;
    }
}

float Animation::get_sec() {
    return (1.0f * get_now_ms()) / 1000.0f;
}

void Animation::play(uint32_t start_frame) {
    _state = ANIMATION_STATE_PLAYING;
    _begin_ts = get_now_ms();
    for (auto channel : _channels) {
        channel->set_offset(start_frame);
    }
}

void Animation::pause() {
    _state = ANIMATION_STATE_PAUSING;
}

void Animation::stop() {
    _state = ANIMATION_STATE_DEFAULT;
    _begin_ts = 0;
    for (auto channel : _channels) {
        channel->reset();
    }
}
