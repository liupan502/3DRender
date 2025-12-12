//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace zr{
    namespace core {
        class Device;
        class CommandBuffer;
        class Queue{
        public:
            Queue(VkQueue vk_queue) : _vk_queue(vk_queue) {

            }
            void submit_cmd(std::shared_ptr<CommandBuffer> cmd_buf);
            // inline std::shared_ptr<Device> get_device() const { return _device;};
            inline uint32_t get_family_idx() const { return _family_idx;};
            inline uint32_t get_idx() const { return _idx; };
            inline VkBool32 can_present() const { return _can_present;};
            inline VkQueue get() { return _vk_queue;};
            inline const VkQueueFamilyProperties& get_properties() const { return _preoerties;}
            virtual ~Queue();
        private:
            
            VkQueue _vk_queue{VK_NULL_HANDLE};
            uint32_t _family_idx{0};
            uint32_t _idx{0};
            VkBool32 _can_present{VK_FALSE};
            VkQueueFamilyProperties _preoerties{};
        };
    }
}
