//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/image_view.h>
#include <core/image.h>
#include <core/vk_common.h>
#include <core/device.h>

using namespace zr::core;

ImageView::ImageView( std::shared_ptr<Device> device,
        std::shared_ptr<Image>image, VkFormat fmt, VkImageViewType type,
                     VkImageAspectFlags aspect_flag,
                     uint32_t base_mip_level,
                     uint32_t base_array_layer, uint32_t mip_levels_count,
                     uint32_t array_layers_count) : _device(device){
    _is_valid = true;
    _image = image;
    _fmt = fmt;
    if (_fmt == VK_FORMAT_UNDEFINED) {
        _fmt = image->get_format();
    }
    _type = type;
    _subresource_range.baseMipLevel = base_mip_level;
    _subresource_range.levelCount = mip_levels_count;
    _subresource_range.baseArrayLayer = base_array_layer;
    _subresource_range.layerCount = array_layers_count;
    _subresource_range.aspectMask = aspect_flag;

    VkImageViewCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.image = *(_image->get());
    ci.viewType = _type;
    ci.format = _fmt;
    ci.subresourceRange = _subresource_range;

    auto ret = vkCreateImageView(_image->get_device(), &ci, nullptr, &_vk_image_view);

    if (ret != VK_SUCCESS) {
        _is_valid = false;
    }

}

ImageView::~ImageView() {
    vkDestroyImageView(_device->get_device(), _vk_image_view, nullptr);
}
