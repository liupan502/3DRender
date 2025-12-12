//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/image.h>
#include <core/device.h>
#include <core/vk_common.h>
#include <core/buffer.h>
#include <core/command_pool.h>
#include <core/command_buffer.h>
#include <utils/log.h>
using namespace  zr::core;

const VkDevice& Image::get_device() const {
    return _device->get_device();
}

Image::Image(std::shared_ptr<Device> device, const VkExtent3D &extent, VkFormat fmt, VkImageUsageFlags img_usage,
             VkMemoryPropertyFlagBits mem_usage, VkSampleCountFlagBits sample_count,
             uint32_t mip_levels_count, uint32_t array_layers_count, VkImageTiling tiling,
             VkImageCreateFlags flags, VkImageType img_type,
             uint32_t num_queue_families, uint32_t *queue_families) : _device(device){
    _fmt = fmt;
    _img_type = img_type;
    _extent = extent;
    _sample_count = sample_count;
    _usage = img_usage;
    _array_layer_count = array_layers_count;
    _tiling = tiling;

    _subresource.mipLevel = mip_levels_count;
    _subresource.arrayLayer = array_layers_count;

    VkImageCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.tiling = _tiling;
    ci.flags = flags;
    ci.imageType = _img_type;
    ci.format = _fmt;
    ci.extent = _extent;
    ci.mipLevels = mip_levels_count;
    ci.arrayLayers = array_layers_count;
    ci.samples = sample_count;
    ci.usage = img_usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = num_queue_families;
    ci.pQueueFamilyIndices = queue_families;

    CALL_VK(vkCreateImage(_device->get_device(), &ci, nullptr, &_vk_image));

    VkMemoryRequirements _mem_req;
    vkGetImageMemoryRequirements(_device->get_device(), _vk_image, &_mem_req);
    VkMemoryAllocateInfo _mem_alloc_info;
    _mem_alloc_info.allocationSize = _mem_req.size;
    _mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    _mem_alloc_info.pNext = nullptr;

    uint32_t mem_flag_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if ((ci.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        mem_flag_bits |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }
    device->map_memory_type_to_idx(_mem_req.memoryTypeBits, mem_flag_bits,
                           &_mem_alloc_info.memoryTypeIndex);
    VkResult result = vkAllocateMemory(_device->get_device(), &_mem_alloc_info, nullptr, &_vk_image_memory);
    if (result != VK_SUCCESS) {
        LOGD("vkAllocateMemory FAILED , allocationSize-> %lu,_extent height -> %u,width -> %u,depth -> %u,"
             "mip_levels_count -> %u"
             "array_layers_count -> %u,",
             _mem_alloc_info.allocationSize,
             extent.height,
             _extent.width,
             _extent.depth,
             mip_levels_count,
             array_layers_count)
    }
    vkBindImageMemory(_device->get_device(), _vk_image, _vk_image_memory, 0);
}

void Image::set_layout(VkCommandBuffer cmd_buf, VkImageLayout old_img_layout,
                       VkImageLayout new_img_layout, VkPipelineStageFlags src_stages,
                       VkPipelineStageFlags dst_stages) {
    return;
    VkImageMemoryBarrier img_mem_bar;
    
    img_mem_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_mem_bar.pNext = nullptr;
    img_mem_bar.srcAccessMask = 0;
    img_mem_bar.dstAccessMask = 0;
    img_mem_bar.oldLayout = old_img_layout;
    img_mem_bar.newLayout = new_img_layout;
    img_mem_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_mem_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_mem_bar.image = _vk_image;
    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    img_mem_bar.subresourceRange = range;
    

    switch(old_img_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            img_mem_bar.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            img_mem_bar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            img_mem_bar.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;
        default:
            break;
    };

    switch(new_img_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            img_mem_bar.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            img_mem_bar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            img_mem_bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            img_mem_bar.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            img_mem_bar.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            img_mem_bar.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;
        default:
            break;
    }
    vkCmdPipelineBarrier(cmd_buf, src_stages, dst_stages, 0, 0, nullptr, 0, nullptr, 1,
                         &img_mem_bar);
}

void Image::update_data(unsigned char* data, uint32_t size, uint8_t mip_level,
                        uint8_t base_layer, VkFormat fmt) {
    Buffer stage_buf(_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stage_buf.update(data, size);
    auto fn = [&](std::shared_ptr<CommandBuffer> cmd_buf) {
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, mip_level, 1, base_layer, 1);
        auto region = create_buffer_imgage_copy(mip_level, base_layer);
        copy_buffer_to_image(stage_buf.get(), cmd_buf, {region});
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buf, mip_level, 1, base_layer, 1);
    };
    _device->get_cmd_pool()->execute_single_cmd(fn);
}

void Image::update_data(unsigned char *data, uint32_t size, uint8_t base_layer,
                        VkFormat fmt) {
    Buffer stage_buf(_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stage_buf.update(data, size);
    if (_subresource.mipLevel == 1) {
        auto fn = [&](std::shared_ptr<CommandBuffer> cmd_buf) {
            transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, 0, 1, base_layer, 1);
            auto region = create_buffer_imgage_copy(0, base_layer);
            copy_buffer_to_image(stage_buf.get(), cmd_buf, {region});
            transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buf, 0, 1, base_layer, 1);
        };
        _device->get_cmd_pool()->execute_single_cmd(fn);
        return;
    }
    else {
        auto fn = [&](std::shared_ptr<CommandBuffer> cmd_buf) {
            transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, 0, 1, base_layer, 1);
            auto region = create_buffer_imgage_copy(0, base_layer);
            copy_buffer_to_image(stage_buf.get(), cmd_buf, {region});
            transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd_buf, 0, 1, base_layer, 1);
            uint16_t width = _extent.width;
            uint16_t height = _extent.height;
            for (uint8_t i = 1; i < _subresource.mipLevel; i++) {

                BlitParams src_blit_params{base_layer, (uint16_t)(i - 1), (uint16_t)(width >> (i - 1)), (uint16_t)(height >> (i - 1)), 1};
                BlitParams dst_blit_params{base_layer, (uint16_t)(i), (uint16_t)(width >> (i)), (uint16_t)(height >> (i)), 1};
                blit(_vk_image, src_blit_params, dst_blit_params, cmd_buf);
            }
            transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buf,
                                             0, _subresource.mipLevel, base_layer, 1);
        };
        _device->get_cmd_pool()->execute_single_cmd(fn);
    }

}

void Image::transition_image_layout(VkFormat fmt, VkImageLayout old_layout,
                                    VkImageLayout new_layout, uint8_t base_mip_level , uint8_t mip_level_count) {
    auto fn = [&, fmt, old_layout, new_layout](std::shared_ptr<CommandBuffer> cmd_buf) {
            transition_image_layout_internal(fmt, old_layout, new_layout,
                                             cmd_buf, base_mip_level, mip_level_count, 0, 1);
    };

    _device->get_cmd_pool()->execute_single_cmd(fn);
}

void Image::transition_image_layout_internal(VkFormat fmt, VkImageLayout old_layout, VkImageLayout new_layout,
                                             std::shared_ptr<CommandBuffer> cmd_buf,
                                             uint16_t base_mip_level, uint16_t mip_level_count,
                                             uint8_t base_layer, uint16_t layer_count) {


        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = _vk_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = base_mip_level;
        barrier.subresourceRange.levelCount = mip_level_count;
        barrier.subresourceRange.baseArrayLayer = base_layer;
        barrier.subresourceRange.layerCount = layer_count;

        VkPipelineStageFlags src_pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        VkPipelineStageFlags dst_pipe_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            src_pipe_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_pipe_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
                 new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            bool has_stencil_comp = (fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT);
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (has_stencil_comp) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            src_pipe_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_pipe_stage_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            src_pipe_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_pipe_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            src_pipe_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_pipe_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                 new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_pipe_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_pipe_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        }
        else {

        }

        vkCmdPipelineBarrier(cmd_buf->get(), src_pipe_stage_flags,
                             dst_pipe_stage_flags, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);
}

void Image::copy_buffer_to_image(VkBuffer buf, std::shared_ptr<CommandBuffer> cmd_buf,
                                 std::vector<VkBufferImageCopy> regions) {
        vkCmdCopyBufferToImage(cmd_buf->get(), buf, _vk_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
}

VkBufferImageCopy Image::create_buffer_imgage_copy(
        uint8_t mip_level, uint8_t base_layer) const {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = mip_level;
    region.imageSubresource.baseArrayLayer = base_layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent.width = (_extent.width >> mip_level);
    region.imageExtent.height = (_extent.height >> mip_level);
    uint16_t depth = (_extent.depth >> mip_level) ? (_extent.depth >> mip_level) : 1;
    region.imageExtent.depth = depth;
    return region;
}

Image::~Image() {
    if (_vk_image_memory) {
        vkFreeMemory(_device->get_device(), _vk_image_memory, nullptr);
        vkDestroyImage(_device->get_device(), _vk_image, nullptr);
    }
}

void Image::blit(VkImage src_vk_image,
                 const BlitParams& src_blit_params, const BlitParams& dst_blit_params,
                 std::shared_ptr<CommandBuffer> cmd_buf) {
        VkImageBlit vk_image_blit{};

        vk_image_blit.srcSubresource.layerCount = 1;
        vk_image_blit.srcSubresource.baseArrayLayer = src_blit_params.base_layer;
        vk_image_blit.srcSubresource.mipLevel = src_blit_params.mip_level;
        vk_image_blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vk_image_blit.srcOffsets[0].x = src_blit_params.width;
        vk_image_blit.srcOffsets[0].y = src_blit_params.height;
        vk_image_blit.srcOffsets[0].z = 0;
        // vk_image_blit.srcOffsets[1].x = src_blit_params.width;
        // vk_image_blit.srcOffsets[1].y = src_blit_params.height;
        vk_image_blit.srcOffsets[1].z = 1;

        vk_image_blit.dstSubresource.layerCount = 1;
        vk_image_blit.dstSubresource.baseArrayLayer = dst_blit_params.base_layer;
        vk_image_blit.dstSubresource.mipLevel = dst_blit_params.mip_level;
        vk_image_blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vk_image_blit.dstOffsets[0].x = dst_blit_params.width;
        vk_image_blit.dstOffsets[0].y = dst_blit_params.height;
        vk_image_blit.dstOffsets[0].z = 0;
        // vk_image_blit.dstOffsets[1].x = dst_blit_params.width;
        // vk_image_blit.dstOffsets[1].y = dst_blit_params.height;
        vk_image_blit.dstOffsets[1].z = 1;
        blit_internal(src_vk_image, vk_image_blit, cmd_buf, dst_blit_params.mip_level);
}

void Image::blit_internal(VkImage src_vk_img, VkImageBlit vk_image_blit,
                          std::shared_ptr<CommandBuffer> cmd_buf, uint8_t mip_level) {
    transition_image_layout_internal(_fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, mip_level, 1,
                                     vk_image_blit.dstSubresource.baseArrayLayer, 1);
    vkCmdBlitImage(cmd_buf->get(), src_vk_img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    _vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_image_blit, VK_FILTER_LINEAR);
    transition_image_layout_internal(_fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd_buf, mip_level, 1,
                                     vk_image_blit.dstSubresource.baseArrayLayer, 1);
}
