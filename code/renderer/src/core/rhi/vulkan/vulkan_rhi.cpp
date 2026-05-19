#include <rhi/vulkan/vulkan_rhi.h>
#include <rhi/vulkan/vulkan_buffer.h>
#include <rhi/vulkan/vulkan_texture.h>
#include <rhi/vulkan/vulkan_shader_module.h>
#include <rhi/vulkan/vulkan_pipeline.h>
#include <rhi/vulkan/vulkan_render_target.h>

#ifdef PLATFORM_ANDROID
#include <android/native_window.h>
#elif PLATFORM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace rhi {

VulkanRHI::~VulkanRHI()
{
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

void VulkanRHI::update_texture(TextureRef tex, void* data, uint32_t len， 
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
    return std::make_shared<vulkan::VulkanGraphicsPipeline>(info);
}

}