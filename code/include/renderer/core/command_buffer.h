//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <memory>

#include <vulkan/vulkan.h>

namespace zr {
    namespace core {
        class Device;
        class CommandPool;
        class CommandBuffer {
        public:
            CommandBuffer(std::shared_ptr<Device> device, CommandPool* cmd_pool,
                          VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            CommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> cmd_pool,
                          VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            inline VkCommandBuffer get() const { return _vk_command_buffer;};

            void begin();
            void end();

            virtual ~CommandBuffer();
        private:
            std::shared_ptr<Device> _device;
            VkCommandPool _vk_command_pool{VK_NULL_HANDLE};
            VkCommandBuffer _vk_command_buffer{VK_NULL_HANDLE};
        };
    }
}
