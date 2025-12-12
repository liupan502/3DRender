//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>

bool is_depth_only_format(VkFormat format) {
    bool ret = format == VK_FORMAT_D16_UNORM ||
                format == VK_FORMAT_D32_SFLOAT;
    return ret;
}

bool is_depth_stencil_format(VkFormat format) {
    bool ret = format == VK_FORMAT_D16_UNORM_S8_UINT ||
                format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            is_depth_only_format(format);
    return ret;
}