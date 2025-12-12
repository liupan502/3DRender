//
// Created by zhida.ji1 on 2022/8/9.
//
#include <scenegraph/components/sampler.h>
#include <core/core.h>
using namespace zr::core;

Sampler::Sampler(std::shared_ptr<Device>device, SamplerInfo si) : _device(device){

    VkSamplerCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    ci.magFilter = si.mag_filter;
    ci.minFilter = si.min_filter;
    ci.addressModeU = si.amu;
    ci.addressModeV = si.amv;
    ci.addressModeW = si.amw;
    ci.anisotropyEnable = si.anisotropy_enable;
    ci.maxAnisotropy = si.max_anisotropy;
    ci.borderColor = si.border_color;
    ci.unnormalizedCoordinates = si.unnormalized_coordinates;
    ci.compareEnable = si.compare_enable;
    ci.compareOp = si.compare_op;
    ci.mipmapMode = si.mipmap_mode; 
    ci.maxLod = si.max_lod;

    CALL_VK(vkCreateSampler(_device->get_device(),&ci, nullptr, &_vk_sampler));
}

Sampler::~Sampler() {
    vkDestroySampler(_device->get_device(), _vk_sampler, nullptr);
}
