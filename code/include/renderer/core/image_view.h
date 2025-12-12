//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <vulkan/vulkan.h>
#include <memory>

namespace zr {

    namespace core {
        class Image;
        class Device;
        class ImageView{

        public:
            ImageView(
                    std::shared_ptr<Device> device,
                    std::shared_ptr<Image> image,
                      VkFormat fmt = VK_FORMAT_UNDEFINED,
                      VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D,
                      VkImageAspectFlags aspect_flag = VK_IMAGE_ASPECT_COLOR_BIT,
                      uint32_t base_mip_level = 0,
                      uint32_t base_array_layer = 0,
                      uint32_t mip_levels_count = 1,
                      uint32_t array_layers_count = 1);
            ImageView() = default;
            virtual ~ImageView();
            inline VkFormat format() const { return _fmt; };
            inline VkImageSubresourceRange subresource_range() const { return _subresource_range ;};
            inline VkImageViewType type() const { return _type; };
            inline VkImageView get() const { return _vk_image_view;};
        private:
            std::shared_ptr<Image> _image{};
            VkFormat _fmt{};
            VkImageViewType _type{};
            VkImageSubresourceRange _subresource_range{};
            VkImageView _vk_image_view{};
            std::shared_ptr<Device> _device;
            bool _is_valid{false};
        };
    }
}
