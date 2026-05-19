#pragma once

#include <rhi/rhi_resource.h>

namespace rhi{
    namespace vulkan{
        class VulkanRenderTarget : public RenderTarget {
            public:
            VulkanRenderTarget(std::shared_ptr<zr::RenderContext> context,
                const RenderTargetCreateInfo& info);
        };
    };
};