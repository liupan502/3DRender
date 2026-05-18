#include <rhi/vulkan/vulkan_shader_module.h>
#include <core/vk_common.h>
#include <shaderc/shaderc.hpp>
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

    if (info.is_bin) {
        shader_module_ci.codeSize = info.len;
        shader_module_ci.pCode = reinterpret_cast<const uint32_t*>(info.content);
    } else {
        shaderc::Compiler compiler;

        shaderc_shader_kind kind = shaderc_vertex_shader;
        if (info.type == ShaderModuleType::SMT_FRAGMENT) {
            kind = shaderc_fragment_shader;
        }

        const char* source = reinterpret_cast<const char*>(info.content);
        size_t source_size = info.len;

        shaderc::CompilationResult<uint32_t> result =
            compiler.CompileGlslToSpv(source, source_size, kind, "shader");

        if (result.GetNumErrors() > 0) {
            assert(false && "GLSL compilation failed");
            return;
        }

        const uint32_t* begin = result.cbegin();
        const uint32_t* end = result.cend();
        shader_module_ci.codeSize = (end - begin) * sizeof(uint32_t);
        shader_module_ci.pCode = begin;
    }

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
