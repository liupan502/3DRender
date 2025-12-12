//
// Created by zhida.ji1 on 2022/8/9.
//

#include <core/core.h>
using namespace zr::core;

void Queue::submit_cmd(std::shared_ptr<CommandBuffer> cmd_buf) {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    VkCommandBuffer vk_cmd_buf = cmd_buf->get();
    submit_info.pCommandBuffers = &vk_cmd_buf;
    CALL_VK(vkQueueSubmit(_vk_queue, 1, &submit_info, VK_NULL_HANDLE));

    CALL_VK(vkQueueWaitIdle(_vk_queue));
}

Queue::~Queue() {

}
