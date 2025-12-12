//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>
#include "queue.h"
#include <functional>
#include <memory>
namespace zr {
    namespace core {
        class Device;
        class CommandBuffer;
        class CommandPool {
        public:
            CommandPool(std::shared_ptr<Device> device);
            virtual ~CommandPool();
            inline VkCommandPool get() const { return _vk_command_pool;};

            void execute_single_cmd(std::function<void(std::shared_ptr<CommandBuffer> cmd_buf)> do_cmd);
        private:
            std::shared_ptr<CommandBuffer>  begin_single_time_commands();
            void end_single_time_commands(std::shared_ptr<CommandBuffer> cmd_buf);

        private:
            std::shared_ptr<Device> _device ;
            VkCommandPool _vk_command_pool{VK_NULL_HANDLE};
        };
    }
}
