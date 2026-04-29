#include <rhi/rhi.h>
#include <rhi/vulkan/vulkan_rhi.h>

namespace rhi {

RHI* rhi_instance = nullptr;

void init(const void* window_handle)
{
    rhi_instance = new VulkanRHI();
    rhi_instance->init(window_handle);
}

}