#include <rhi/vulkan/vulkan_shader_module.h>
#include <core/vk_common.h>
#include <core/device.h>
#include <cassert>

namespace rhi {
namespace vulkan {

VulkanShaderModule::VulkanShaderModule(std::shared_ptr<zr::RenderContext> context,
                                       const ShaderModuleCreateInfo& info)
    : ShaderModule(info), _context(context)  {
    
    auto device = _context->get_device();
    VkShaderModuleCreateInfo shader_module_ci{};
    shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_ci.pNext = nullptr;
    shader_module_ci.flags = 0;

    assert(info.is_bin && "Only binary shader modules are supported");
    shader_module_ci.codeSize = info.len;
    shader_module_ci.pCode = reinterpret_cast<const uint32_t*>(info.content);

    CALL_VK(vkCreateShaderModule(device->get_device(), &shader_module_ci,
                                  nullptr, &_vk_shader_module));
}

VulkanShaderModule::~VulkanShaderModule() {
    if (_vk_shader_module != VK_NULL_HANDLE) {
        auto device = _context->get_device();
        vkDestroyShaderModule(device->get_device(), _vk_shader_module, nullptr);
    }
}

} // namespace vulkan
} // namespace rhi
