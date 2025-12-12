//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>
using namespace  zr::core;

RenderPass::RenderPass(std::shared_ptr<Device> device,
                       VkExtent2D display_size,
                       //std::shared_ptr<Swapchain> swapchain,
                       const VkRenderPassCreateInfo& ci) : _device(device){
    _display_size = display_size;
    _render_area.extent = _display_size;
    _render_area.offset = {(int32_t)0, (int32_t)0};
    
    CALL_VK(vkCreateRenderPass(_device->get_device(), & ci, nullptr, &_vk_render_pass));
}

RenderPass::RenderPass(std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain,
                       bool need_enable_depath_test,
                       bool need_enable_msaa,
                       VkSampleCountFlagBits msaa_samples) : _device(device), 
                            _enabled_depth_test(need_enable_depath_test),
                            _enabled_msaa(need_enable_msaa){
    _display_size = swapchain->get_display_size();
    _render_area.extent = _display_size;
    _render_area.offset = {(int32_t)0, (int32_t)0};
    std::vector<VkAttachmentDescription> attachment_descs;

    VkSubpassDescription subpass_desc{};

    VkFormat format = swapchain->get_suitable_format();
    if (need_enable_msaa) {
        enable_msaa(msaa_samples, format, attachment_descs, subpass_desc);
        create_msaa_attachment(msaa_samples, swapchain->get_display_size());
    }

    if (need_enable_depath_test) {
        enable_depth(msaa_samples, VK_FORMAT_D32_SFLOAT, attachment_descs, subpass_desc);
        create_depth_attachment(msaa_samples, swapchain->get_display_size());
    }

    enable_color(VK_SAMPLE_COUNT_1_BIT, format, attachment_descs, subpass_desc);



    VkSubpassDependency dependency;
    
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = attachment_descs.size();
    ci.pAttachments = attachment_descs.data();
    ci.subpassCount = 1;
    ci.pSubpasses = &subpass_desc;
    ci.dependencyCount = 1;
    ci.pDependencies = &dependency;
    CALL_VK(vkCreateRenderPass(_device->get_device(), & ci, nullptr, &_vk_render_pass));

    _framebuffers.clear();
    auto display_image_views = swapchain->get_display_image_views();
    for (uint32_t idx = 0; idx < display_image_views.size(); idx++) {
        std::vector<std::shared_ptr<ImageView>> attachments;
        if (need_enable_msaa) {
            attachments.push_back(_msaa_image_view);
        }

        if (need_enable_depath_test) {
            attachments.push_back(_depth_image_view);
        }
        attachments.push_back(display_image_views[idx]);
        _framebuffers.push_back(std::make_shared<Framebuffer>(_device, this,
                                                              swapchain->get_display_size(), attachments));
    }
}

void RenderPass::enable_depth(VkSampleCountFlagBits samples, VkFormat format,
                              std::vector<VkAttachmentDescription>& descs, VkSubpassDescription& subpass_desc) {

    VkAttachmentDescription attachment{};
    if (_enabled_msaa) {
        attachment.samples = samples;
    }
    else {
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    }
    attachment.format = format;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descs.push_back(attachment);

    _depth_ref = VkAttachmentReference{};
    _depth_ref.attachment = descs.size() - 1;
    _depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    subpass_desc.pDepthStencilAttachment = &_depth_ref;

}

void RenderPass::enable_msaa(VkSampleCountFlagBits samples, VkFormat format,
                             std::vector<VkAttachmentDescription>& descs, VkSubpassDescription& subpass_desc) {
    VkAttachmentDescription attachment{};
    attachment.samples = samples;
    attachment.format = format;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descs.push_back(attachment);

    _msaa_ref = VkAttachmentReference{};
    _msaa_ref.attachment = descs.size() - 1;
    _msaa_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass_desc.pColorAttachments = &_msaa_ref;
}

void RenderPass::enable_color(VkSampleCountFlagBits samples, VkFormat format,
                              std::vector<VkAttachmentDescription> &descs,
                              VkSubpassDescription &subpass_desc) {
    VkAttachmentDescription attachment{};
    attachment.samples = samples;
    attachment.format = format;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descs.push_back(attachment);

    _color_ref = VkAttachmentReference{};
    _color_ref.attachment = descs.size() - 1;
    _color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (_enabled_msaa) {
        subpass_desc.pResolveAttachments = &_color_ref;
    }
    else {
        subpass_desc.pColorAttachments = &_color_ref;
    }
    // subpass_desc.pColorAttachments = &_color_ref;
    // subpass_desc.pResolveAttachments = &_color_ref;
    subpass_desc.colorAttachmentCount = 1;
}

void RenderPass::create_msaa_attachment(VkSampleCountFlagBits samples, VkExtent2D extent) {
    VkExtent3D extent3D{};
    extent3D.width = extent.width;
    extent3D.height = extent.height;
    extent3D.depth = 1;
    _msaa_image = std::make_shared<Image>(_device,
                                         extent3D,
                                         VK_FORMAT_R8G8B8A8_SRGB,
                                         VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                         samples);

    _msaa_image_view = std::make_shared<ImageView>(_device, _msaa_image, VK_FORMAT_R8G8B8A8_SRGB);

    // _msaa_image->transition_image_layout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void RenderPass::create_depth_attachment(VkSampleCountFlagBits samples, VkExtent2D extent) {
    VkExtent3D extent3D{};
    extent3D.width = extent.width;
    extent3D.height = extent.height;
    extent3D.depth = 1;

    _depth_image = std::make_shared<Image>(_device, extent3D,
                                           VK_FORMAT_D32_SFLOAT, // for simple
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                           samples);

    _depth_image_view = std::make_shared<ImageView>(_device, _depth_image, VK_FORMAT_D32_SFLOAT,
                                                    VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
    // _depth_image->set_layout()
    _depth_image->transition_image_layout(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void RenderPass::begin(std::shared_ptr<CommandBuffer> cmd_buf,
                       std::shared_ptr<Framebuffer> framebuffer,
                       std::vector<VkClearValue> clear_vals) {
    VkRenderPassBeginInfo renderpass_begin_info{};
    
    renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.pNext = nullptr;
    renderpass_begin_info.renderPass = _vk_render_pass;
    renderpass_begin_info.framebuffer = framebuffer->get();
    renderpass_begin_info.renderArea = _render_area;
    renderpass_begin_info.clearValueCount = clear_vals.size();
    renderpass_begin_info.pClearValues = clear_vals.data();
    
    vkCmdBeginRenderPass(cmd_buf->get(), &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(std::shared_ptr<CommandBuffer> cmd_buf) {
    vkCmdEndRenderPass(cmd_buf->get());
}

void RenderPass::next(std::shared_ptr<CommandBuffer> cmd_buf) {
    vkCmdNextSubpass(cmd_buf->get(), VK_SUBPASS_CONTENTS_INLINE);
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(_device->get_device(), _vk_render_pass, nullptr);
}

 void RenderPass::set_render_area(int offset_x, int offset_y, int width, int height) {
    if (offset_x + width > _display_size.width || 
        offset_y + height > _display_size.height) {
            return;
    }
    _render_area.extent.width = width;
    _render_area.extent.height = height;
    _render_area.offset.x = offset_x;
    _render_area.offset.y = offset_y;
 }
