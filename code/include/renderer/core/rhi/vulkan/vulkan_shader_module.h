#pragma once

#include <rhi/rhi_resource.h>
#include <render_context.h>
#include <vulkan/vulkan.h>
#include <memory>

namespace rhi{
    namespace vulkan {
        class VulkanShaderModule : public ShaderModule {
        public:
            VulkanShaderModule(std::shared_ptr<zr::RenderContext> context,
                               const ShaderModuleCreateInfo& info);
            ~VulkanShaderModule() override;

            VulkanShaderModule(const VulkanShaderModule&) = delete;
            VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;

            inline VkShaderModule get() const { return _vk_shader_module; }

        private:
            std::shared_ptr<zr::RenderContext> _context;
            VkShaderModule _vk_shader_module{VK_NULL_HANDLE};
        };
    };
};