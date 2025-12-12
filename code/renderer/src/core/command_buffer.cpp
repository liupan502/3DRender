//
// Created by zhida.ji1 on 2022/8/9.
//

#include <core/command_buffer.h>
#include <core/command_pool.h>
#include <core/vk_common.h>
#include <core/device.h>
using namespace zr::core;

CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(_device->get_device(), _vk_command_pool, 1, &_vk_command_buffer);
}

CommandBuffer::CommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> cmd_pool, VkCommandBufferLevel level) :
    _device(device),  _vk_command_pool(cmd_pool->get()){
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = cmd_pool->get();
    alloc_info.level = level;

    CALL_VK(vkAllocateCommandBuffers(_device->get_device(), &alloc_info, &_vk_command_buffer));
}

CommandBuffer::CommandBuffer(std::shared_ptr<Device> device, CommandPool* cmd_pool, VkCommandBufferLevel level) :
        _device(device), _vk_command_pool(cmd_pool->get()){
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = cmd_pool->get();
    alloc_info.level = level;

    CALL_VK(vkAllocateCommandBuffers(_device->get_device(), &alloc_info, &_vk_command_buffer));
}

void CommandBuffer::begin() {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(_vk_command_buffer, &begin_info);
}

void CommandBuffer::end() {
    vkEndCommandBuffer(_vk_command_buffer);
}