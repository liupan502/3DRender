#include <rhi/vulkan/vulkan_render_target.h>

namespace rhi {
namespace vulkan {

VulkanRenderTarget::VulkanRenderTarget(std::shared_ptr<zr::RenderContext> context,
                                       const RenderTargetCreateInfo& info)
    : RenderTarget(info) {
    (void)context;
}

} // namespace vulkan
} // namespace rhi
