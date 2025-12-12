//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>

#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    assert(false);                                                    \
  }

bool is_depth_only_format(VkFormat format);

bool is_depth_stencil_format(VkFormat format);

