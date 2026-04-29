#pragma once
#include "rhi/rhi.h"
#include <render_context.h>
#include <memory>

namespace rhi { 
    class VulkanRHI : public RHI {
    public:
        VulkanRHI() = default;
        ~VulkanRHI() override;
        void init(const void* window) override;
        void destroy() override;

        BufferRef create_buffer(const BufferCreateInfo& info) override;

        void update_buffer(BufferRef buf, void* data, uint32_t len, uint32_t offset) override;

        TextureRef create_texture(const TextureCreateInfo& info) override;

        void update_texture(TextureRef tex, void* data, uint32_t len) override;
    private:
        std::shared_ptr<zr::RenderContext> _context;
    };
}