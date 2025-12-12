//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <memory>
#include <vulkan/vulkan.h>

namespace zr {
    namespace core {
        class Device;

        struct SamplerInfo {
            VkFilter mag_filter = VK_FILTER_LINEAR;
            VkFilter min_filter = VK_FILTER_LINEAR;
            VkSamplerAddressMode amu = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode amv = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode amw = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkBool32 anisotropy_enable = VK_FALSE;
            float max_anisotropy = 1.0f;
            VkBorderColor border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            VkBool32 unnormalized_coordinates = VK_FALSE;
            VkBool32 compare_enable = VK_FALSE;
            VkCompareOp compare_op = VK_COMPARE_OP_ALWAYS;
            VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            float max_lod = 4.0f;
        };

        class Sampler {
        public:
            Sampler(std::shared_ptr<Device> device, SamplerInfo samper_info = SamplerInfo());
            VkSampler get() const { return _vk_sampler;};
            virtual ~Sampler();
        private:
            VkSampler _vk_sampler{VK_NULL_HANDLE};
            std::shared_ptr<Device> _device;
        };
    }
}
