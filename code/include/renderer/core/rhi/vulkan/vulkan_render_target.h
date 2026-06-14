#pragma once

#include <rhi/rhi_resource.h>
#include <render_context.h>

namespace rhi{
    namespace vulkan{
        class VulkanRenderTarget : public RenderTarget {
            public:
            VulkanRenderTarget(std::shared_ptr<zr::RenderContext> context,
                const RenderTargetCreateInfo& info);

            inline const RenderTargetCreateInfo& get_ci() const { return _ci; }
        };
    };
};