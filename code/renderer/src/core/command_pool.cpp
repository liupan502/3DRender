//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/command_pool.h>
#include <core/device.h>
#include <core/command_buffer.h>
#include <core/vk_common.h>
using namespace zr::core;

CommandPool::~CommandPool() {
    vkDestroyCommandPool(_device->get_device(), _vk_command_pool, nullptr);
}

CommandPool::CommandPool(std::shared_ptr<Device> device) : _device(device) {
    VkCommandPoolCreateInfo cmd_pool_ci;
    
    cmd_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_ci.pNext = nullptr;
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_ci.queueFamilyIndex = _device->get_graphic_queue_family_idx();
    
    CALL_VK(vkCreateCommandPool(_device->get_device(), &cmd_pool_ci, nullptr, &_vk_command_pool));
}

std::shared_ptr<CommandBuffer>  CommandPool::begin_single_time_commands() {
    std::shared_ptr<CommandBuffer> cmd_buf = std::make_shared<CommandBuffer>(_device, this);
    cmd_buf->begin();
    return cmd_buf;
}

void CommandPool::end_single_time_commands(std::shared_ptr<CommandBuffer> cmd_buf) {
    cmd_buf->end();
    _device->get_cmd_queue()->submit_cmd(cmd_buf);
}

void CommandPool::execute_single_cmd(
        std::function<void(std::shared_ptr<CommandBuffer>)> do_cmd) {
    std::shared_ptr<CommandBuffer> cmd_buf = begin_single_time_commands();
    do_cmd(cmd_buf);
    end_single_time_commands(cmd_buf);
}
