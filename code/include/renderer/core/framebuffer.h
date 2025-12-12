//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace zr {
    namespace core {
        class RenderPass;
        class Device;
        class ImageView;
        class Framebuffer {
        public:
            Framebuffer() = default;
            Framebuffer(std::shared_ptr<Device> device, RenderPass* render_pass,
                        VkExtent2D extent, std::vector<std::shared_ptr<ImageView>> attchments);

            inline VkFramebuffer get() const { return _vk_frame_buffer;};

            virtual ~Framebuffer();
        private:
            VkFramebuffer _vk_frame_buffer{};
            std::shared_ptr<Device> _device{nullptr};
            VkExtent2D _extent{};
        };
    }
}
