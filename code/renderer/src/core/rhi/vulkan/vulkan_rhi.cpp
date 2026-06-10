#include <rhi/vulkan/vulkan_rhi.h>
#include <rhi/vulkan/vulkan_buffer.h>
#include <rhi/vulkan/vulkan_texture.h>
#include <rhi/vulkan/vulkan_shader_module.h>
#include <rhi/vulkan/vulkan_pipeline.h>
#include <rhi/vulkan/vulkan_render_target.h>
#include <rhi/vulkan/vulkan_descriptor.h>
#include <core/device.h>
#include <core/vk_common.h>
#include <core/command_pool.h>
#include <core/command_buffer.h>

#ifdef PLATFORM_ANDROID
#include <android/native_window.h>
#elif PLATFORM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace rhi {

namespace {

VkFormat color_format_to_vk(ColorFormat fmt) {
    switch (fmt) {
        case ColorFormat::R8G8B8A8_UNORM:   return VK_FORMAT_R8G8B8A8_UNORM;
        case ColorFormat::R8G8B8A8_SRGB:    return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::R8G8B8A8_SNORM:   return VK_FORMAT_R8G8B8A8_SNORM;
        case ColorFormat::R8G8B8A8_UINT:    return VK_FORMAT_R8G8B8A8_UINT;
        case ColorFormat::R8G8B8A8_SINT:    return VK_FORMAT_R8G8B8A8_SINT;
        case ColorFormat::R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ColorFormat::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ColorFormat::D32_SFLOAT:       return VK_FORMAT_D32_SFLOAT;
        case ColorFormat::B8G8R8A8_SRGB:    return VK_FORMAT_B8G8R8A8_SRGB;
        default:                            return VK_FORMAT_UNDEFINED;
    }
}

VkSampleCountFlagBits sample_count_to_vk(SampleCount sc) {
    switch (sc) {
        case SC_COUNT_1: return VK_SAMPLE_COUNT_1_BIT;
        case SC_COUNT_2: return VK_SAMPLE_COUNT_2_BIT;
        case SC_COUNT_4: return VK_SAMPLE_COUNT_4_BIT;
        case SC_COUNT_8: return VK_SAMPLE_COUNT_8_BIT;
        default:         return VK_SAMPLE_COUNT_1_BIT;
    }
}

VkAttachmentLoadOp load_op_to_vk(AttachmentLoadOp op) {
    switch (op) {
        case ALO_LOAD:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case ALO_CLEAR:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case ALO_DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:           return VK_ATTACHMENT_LOAD_OP_LOAD;
    }
}

VkAttachmentStoreOp store_op_to_vk(AttachmentStoreOp op) {
    switch (op) {
        case ASO_STORE:     return VK_ATTACHMENT_STORE_OP_STORE;
        case ASO_DONT_CARE: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:            return VK_ATTACHMENT_STORE_OP_STORE;
    }
}

} // anonymous namespace

VulkanRHI::~VulkanRHI()
{
}

void VulkanRHI::begin_frame() {
    auto device = _context->get_device();
    _cmd_buf = device->get_cmd_pool()->get_available_cmd_buf();
    _cmd_buf->begin();
}

void VulkanRHI::end_frame() {
    _cmd_buf->end();
    _cmd_buf->submit();
    _cmd_buf = nullptr;
}

void VulkanRHI::init(const void* window)
{
    _context = std::make_shared<zr::RenderContext>();
#ifdef PLATFORM_ANDROID
    ANativeWindow* native_window = static_cast<ANativeWindow*>(const_cast<void*>(window));
    _context->init(nullptr, native_window, VK_FORMAT_R8G8B8A8_SRGB);
#elif PLATFORM_GLFW
    GLFWwindow* glfw_window = static_cast<GLFWwindow*>(const_cast<void*>(window));
    _context->init(glfw_window, VK_FORMAT_B8G8R8A8_SRGB);
#else
    (void)window;
#endif
}

void VulkanRHI::destroy()
{
    // TODO: Vulkan cleanup
}

BufferRef VulkanRHI::create_buffer(const BufferCreateInfo& info)
{
    return std::make_shared<vulkan::VulkanBuffer>(_context, info);
}

void VulkanRHI::update_buffer(BufferRef buf, void* data, uint32_t len, uint32_t offset)
{
    auto vk_buf = std::static_pointer_cast<vulkan::VulkanBuffer>(buf);
    vk_buf->update(static_cast<const uint8_t*>(data), len, offset);
}

TextureRef VulkanRHI::create_texture(const TextureCreateInfo& info)
{
    return std::make_shared<vulkan::VulkanTexture>(_context, info);
}

void VulkanRHI::update_texture(TextureRef tex, void* data, uint32_t len, 
            uint32_t base_layer, uint32_t mip_level, bool generated_mip_map)
{
    auto vk_tex = std::static_pointer_cast<vulkan::VulkanTexture>(tex);
    vk_tex->update_data(static_cast<unsigned char*>(data), 
        len, base_layer, mip_level, generated_mip_map);
}

SampleStateRef VulkanRHI::create_sample_state(const SampleStateCreateInfo& info)
{
    return std::make_shared<vulkan::VulkanSampleState>(_context, info);
}

ShaderModuleRef VulkanRHI::create_shader_module(const ShaderModuleCreateInfo& info)
{
    return std::make_shared<vulkan::VulkanShaderModule>(_context, info);
}

GraphicsPipelineRef VulkanRHI::create_graphics_pipeline(const GraphicsPipelineCreateInfo& info) {
    return std::make_shared<vulkan::VulkanGraphicsPipeline>(_context, info);
}

RenderTargetRef VulkanRHI::create_render_target(const RenderTargetCreateInfo& info) {
    return std::make_shared<vulkan::VulkanRenderTarget>(_context, info);
}

DescriptorSetLayoutRef VulkanRHI::create_descriptor_set_layout(const DescriptorSetLayoutCreateInfo& ci) {
    return std::make_shared<vulkan::VulkanDescriptorSetLayout>(ci);
}

DescriptorSetRef VulkanRHI::create_descriptor_set(DescriptorSetLayoutRef layout) {
    return std::make_shared<vulkan::VulkanDescriptorSet>(layout);
}

void VulkanRHI::update_desc_texture(DescriptorSetRef desc, SampleStateRef sampler, TextureRef tex, uint32_t binding_idx) {

}

void VulkanRHI::update_desc_buffer(DescriptorSetRef desc, BufferRef buf, uint32_t binding_idx, uint32_t offset, uint32_t len) {

}

void VulkanRHI::begin_render_pass(RenderTargetRef rt, const RenderPassParams& params) {
    const auto& rt_ci = std::static_pointer_cast<vulkan::VulkanRenderTarget>(rt)->get_ci();
    _current_render_pass = get_or_create_render_pass(rt_ci);
    _current_subpass = 0;

    auto framebuffer = get_or_create_framebuffer(_current_render_pass, rt_ci);

    VkRenderPassBeginInfo rp_begin{};
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass = _current_render_pass;
    rp_begin.framebuffer = framebuffer;
    rp_begin.renderArea.offset.x = params.vp.left;
    rp_begin.renderArea.offset.y = params.vp.top;
    rp_begin.renderArea.extent.width = params.vp.width;
    rp_begin.renderArea.extent.height = params.vp.height;

    VkClearValue clear_values[2];
    uint32_t clear_count = 0;
    for (size_t i = 0; i < rt_ci.color_attachments.size(); i++) {
        clear_values[clear_count].color.float32[0] = params.ci.color.r;
        clear_values[clear_count].color.float32[1] = params.ci.color.g;
        clear_values[clear_count].color.float32[2] = params.ci.color.b;
        clear_values[clear_count].color.float32[3] = params.ci.color.a;
        clear_count++;
    }
    if (rt_ci.depth_attachment.first.fmt != ColorFormat::None) {
        clear_values[clear_count].depthStencil.depth = params.ci.depth;
        clear_values[clear_count].depthStencil.stencil = params.ci.stencil;
        clear_count++;
    }
    rp_begin.clearValueCount = clear_count;
    rp_begin.pClearValues = clear_values;

    vkCmdBeginRenderPass(_cmd_buf->get(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRHI::end_render_pass() {
    _current_render_pass = VK_NULL_HANDLE;
}

VkRenderPass VulkanRHI::get_or_create_render_pass(const RenderTargetCreateInfo& rt_ci) {
    RenderPassKey key;
    for (const auto& color_att : rt_ci.color_attachments) {
        RenderPassKey::AttDesc desc;
        desc.fmt = color_att.first.fmt;
        desc.samples = color_att.first.samples;
        desc.load_op = color_att.first.load_op;
        desc.store_op = color_att.first.store_op;
        key.color_attachments.push_back(desc);
    }
    if (rt_ci.depth_attachment.first.fmt != ColorFormat::None) {
        key.depth_attachment.fmt = rt_ci.depth_attachment.first.fmt;
        key.depth_attachment.samples = rt_ci.depth_attachment.first.samples;
        key.depth_attachment.load_op = rt_ci.depth_attachment.first.load_op;
        key.depth_attachment.store_op = rt_ci.depth_attachment.first.store_op;
    }

    auto it = _render_pass_cache.find(key);
    if (it != _render_pass_cache.end()) {
        return it->second;
    }

    auto device = _context->get_device();

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> color_refs;
    VkAttachmentReference depth_ref{};
    bool has_depth = false;

    for (const auto& color_att : rt_ci.color_attachments) {
        VkAttachmentDescription desc{};
        desc.format = color_format_to_vk(color_att.first.fmt);
        desc.samples = sample_count_to_vk(color_att.first.samples);
        desc.loadOp = load_op_to_vk(color_att.first.load_op);
        desc.storeOp = store_op_to_vk(color_att.first.store_op);
        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments.push_back(desc);

        VkAttachmentReference ref{};
        ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(ref);
    }

    if (rt_ci.depth_attachment.first.fmt != ColorFormat::None) {
        VkAttachmentDescription desc{};
        desc.format = color_format_to_vk(rt_ci.depth_attachment.first.fmt);
        desc.samples = sample_count_to_vk(rt_ci.depth_attachment.first.samples);
        desc.loadOp = load_op_to_vk(rt_ci.depth_attachment.first.load_op);
        desc.storeOp = store_op_to_vk(rt_ci.depth_attachment.first.store_op);
        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(desc);

        depth_ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        has_depth = true;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
    subpass.pColorAttachments = color_refs.data();
    subpass.pDepthStencilAttachment = has_depth ? &depth_ref : nullptr;

    VkRenderPassCreateInfo rp_ci{};
    rp_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    rp_ci.pAttachments = attachments.data();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;
    rp_ci.dependencyCount = 0;
    rp_ci.pDependencies = nullptr;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    CALL_VK(vkCreateRenderPass(device->get_device(), &rp_ci, nullptr, &render_pass));

    _render_pass_cache[key] = render_pass;
    return render_pass;
}

VkFramebuffer VulkanRHI::get_or_create_framebuffer(VkRenderPass render_pass, const RenderTargetCreateInfo& rt_ci) {
    std::vector<VkImageView> image_views;
    for (const auto& color_att : rt_ci.color_attachments) {
        auto vk_tex = std::static_pointer_cast<vulkan::VulkanTexture>(color_att.second);
        image_views.push_back(vk_tex->get_image_view());
    }
    if (rt_ci.depth_attachment.first.fmt != ColorFormat::None) {
        auto vk_tex = std::static_pointer_cast<vulkan::VulkanTexture>(rt_ci.depth_attachment.second);
        image_views.push_back(vk_tex->get_image_view());
    }

    auto key = std::make_pair(render_pass, image_views);
    auto it = _framebuffer_cache.find(key);
    if (it != _framebuffer_cache.end()) {
        return it->second;
    }

    auto device = _context->get_device();

    uint32_t width = 1, height = 1;
    if (!rt_ci.color_attachments.empty()) {
        width = rt_ci.color_attachments[0].first.width;
        height = rt_ci.color_attachments[0].first.height;
    } else if (rt_ci.depth_attachment.first.fmt != ColorFormat::None) {
        width = rt_ci.depth_attachment.first.width;
        height = rt_ci.depth_attachment.first.height;
    }

    VkFramebufferCreateInfo fb_ci{};
    fb_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_ci.renderPass = render_pass;
    fb_ci.attachmentCount = static_cast<uint32_t>(image_views.size());
    fb_ci.pAttachments = image_views.data();
    fb_ci.width = width;
    fb_ci.height = height;
    fb_ci.layers = 1;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    CALL_VK(vkCreateFramebuffer(device->get_device(), &fb_ci, nullptr, &framebuffer));

    _framebuffer_cache[key] = framebuffer;
    return framebuffer;
}

void VulkanRHI::draw(GraphicsPipelineRef pipeline, const RenderPrimitive& primitive, 
    uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) {

    auto vk_pipeline = std::static_pointer_cast<vulkan::VulkanGraphicsPipeline>(pipeline);
    auto key = std::make_pair(vk_pipeline.get(), _current_render_pass);

    VkPipeline vk_pipeline_handle = VK_NULL_HANDLE;
    auto it = _pipeline_cache.find(key);
    if (it != _pipeline_cache.end()) {
        vk_pipeline_handle = it->second;
    } else {
        vk_pipeline_handle = vk_pipeline->get_or_create(_current_render_pass, _current_subpass);
        _pipeline_cache[key] = vk_pipeline_handle;
    }

    vkCmdBindPipeline(_cmd_buf->get(), VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_handle);

    VkDeviceSize offset = 0;
    auto vk_vtx_buf = std::static_pointer_cast<vulkan::VulkanBuffer>(primitive.vtx_buf);
    VkBuffer vk_buf = vk_vtx_buf->get();
    vkCmdBindVertexBuffers(_cmd_buf->get(), 0, 1, &vk_buf, &offset);

    auto vk_idx_buf = std::static_pointer_cast<vulkan::VulkanBuffer>(primitive.idx_buf);
    vkCmdBindIndexBuffer(_cmd_buf->get(), vk_idx_buf->get(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(_cmd_buf->get(), indexCount, instanceCount, indexOffset, 0, 0);
}



}