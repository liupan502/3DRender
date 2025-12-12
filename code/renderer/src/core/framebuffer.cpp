//
// Created by zhida.ji1 on 2022/8/9.
//

#include <core/framebuffer.h>
#include <core/render_pass.h>
#include <core/device.h>
#include <core/image_view.h>
#include <core/vk_common.h>

using namespace zr::core;

Framebuffer::Framebuffer(std::shared_ptr<Device> device, RenderPass* render_pass, VkExtent2D extent,
                         std::vector<std::shared_ptr<ImageView>> attachments) :
                         _device(device), _extent(extent) {
    VkFramebufferCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    ci.renderPass = render_pass->get();
    ci.attachmentCount = attachments.size();
    std::vector<VkImageView> views;
    for (auto& attachment : attachments) {
        views.push_back(attachment->get());
    }
    ci.pAttachments = views.data();
    ci.width = _extent.width;
    ci.height = _extent.height;
    ci.layers = 1;

    CALL_VK(vkCreateFramebuffer(_device->get_device(), &ci, nullptr, &_vk_frame_buffer));
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(_device->get_device(), _vk_frame_buffer, nullptr);
}
