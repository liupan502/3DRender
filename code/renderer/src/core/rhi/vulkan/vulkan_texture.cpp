#include <rhi/vulkan/vulkan_texture.h>
#include <rhi/rhi_resource.h>
#include <core/device.h>
#include <core/vk_common.h>
#include <core/command_pool.h>
#include <core/command_buffer.h>

#include <rhi/vulkan/vulkan_buffer.h>

namespace rhi {
namespace vulkan {

VkFormat VulkanTexture::color_format_to_vk(ColorFormat fmt) const {
    switch (fmt) {
        case ColorFormat::R8G8B8A8_UNORM:   return VK_FORMAT_R8G8B8A8_UNORM;
        case ColorFormat::R8G8B8A8_SRGB:    return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::R8G8B8A8_SNORM:   return VK_FORMAT_R8G8B8A8_SNORM;
        case ColorFormat::R8G8B8A8_UINT:    return VK_FORMAT_R8G8B8A8_UINT;
        case ColorFormat::R8G8B8A8_SINT:    return VK_FORMAT_R8G8B8A8_SINT;
        case ColorFormat::R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ColorFormat::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:                             return VK_FORMAT_UNDEFINED;
    }
}

VkImageType VulkanTexture::texture_type_to_vk_image_type(TextureType type) const {
    switch (type) {
        case TextureType::Texture2D:       return VK_IMAGE_TYPE_2D;
        case TextureType::Texture2DArray:  return VK_IMAGE_TYPE_2D;
        case TextureType::Texture3D:       return VK_IMAGE_TYPE_3D;
        case TextureType::TextureCube:     return VK_IMAGE_TYPE_2D;
        case TextureType::TextureCubeArray:return VK_IMAGE_TYPE_2D;
        default:                           return VK_IMAGE_TYPE_2D;
    }
}

VkImageUsageFlags VulkanTexture::texture_flags_to_vk_usage(TextureCreateFlags flags) const {
    VkImageUsageFlags usage = 0;
    if (flags & static_cast<TextureCreateFlags>(TextureCreateFlagBit::RenderTargetable))
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (flags & static_cast<TextureCreateFlags>(TextureCreateFlagBit::ResolveTargetable))
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (flags & static_cast<TextureCreateFlags>(TextureCreateFlagBit::DepthStencilTargetable))
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (flags & static_cast<TextureCreateFlags>(TextureCreateFlagBit::ShaderResource))
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (flags & static_cast<TextureCreateFlags>(TextureCreateFlagBit::CPUWritable))
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (flags & static_cast<TextureCreateFlags>(TextureCreateFlagBit::InputAttachmentRead))
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    return usage;
}

void VulkanTexture::create_image() {
    auto device = _context->get_device();

    VkImageCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.tiling = _tiling;
    ci.flags = 0;
    ci.imageType = _img_type;
    ci.format = _vk_format;
    ci.extent = _extent;
    ci.mipLevels = _create_info.mip_num;
    ci.arrayLayers = _array_layer_count;
    ci.samples = _sample_count;
    ci.usage = _vk_usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (_create_info.type == TextureType::TextureCube ||
        _create_info.type == TextureType::TextureCubeArray) {
        ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateImage(device->get_vma_allocator(), &ci, &alloc_info,
                   &_vk_image, &_vma_alloc, nullptr);
}

VkImageViewType VulkanTexture::texture_type_to_vk_image_view_type(TextureType type) const {
    switch (type) {
        case TextureType::Texture2D:       return VK_IMAGE_VIEW_TYPE_2D;
        case TextureType::Texture2DArray:  return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TextureType::Texture3D:       return VK_IMAGE_VIEW_TYPE_3D;
        case TextureType::TextureCube:     return VK_IMAGE_VIEW_TYPE_CUBE;
        case TextureType::TextureCubeArray:return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        default:                           return VK_IMAGE_VIEW_TYPE_2D;
    }
}

void VulkanTexture::create_image_view() {
    auto device = _context->get_device();

    VkImageViewCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.image = _vk_image;
    ci.viewType = texture_type_to_vk_image_view_type(_create_info.type);
    ci.format = _vk_format;
    ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.subresourceRange.baseMipLevel = 0;
    ci.subresourceRange.levelCount = _create_info.mip_num;
    ci.subresourceRange.baseArrayLayer = 0;
    ci.subresourceRange.layerCount = _array_layer_count;
    ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    vkCreateImageView(device->get_device(), &ci, nullptr, &_vk_image_view);
}

VulkanTexture::VulkanTexture(std::shared_ptr<zr::RenderContext> context,
                             const TextureCreateInfo& create_info)
    : _context(context) {
    _create_info = create_info;

    _extent.width  = create_info.width;
    _extent.height = create_info.height;
    _extent.depth  = create_info.depth;
    _vk_format     = color_format_to_vk(create_info.format);
    _img_type      = texture_type_to_vk_image_type(create_info.type);
    _vk_usage      = texture_flags_to_vk_usage(create_info.flags);
    _array_layer_count = create_info.layer_num;
    bool cubemap_enabled = create_info.type == TextureType::TextureCube ||
                          create_info.type == TextureType::TextureCubeArray;
    if (cubemap_enabled) {
        _extent.width = _extent.width / 4;
        _extent.height = _extent.height / 3;
        _array_layer_count = 6;
    }
    create_image();
    create_image_view();
}

VulkanTexture::VulkanTexture(std::shared_ptr<zr::RenderContext> context,
                             const TextureCreateInfo& create_info,
                             VkImage existing_image)
    : _context(context), _vk_image(existing_image), _owns_image(false) {
    _create_info = create_info;

    _extent.width  = create_info.width;
    _extent.height = create_info.height;
    _extent.depth  = create_info.depth;
    _vk_format     = color_format_to_vk(create_info.format);
    _img_type      = texture_type_to_vk_image_type(create_info.type);
    _vk_usage      = texture_flags_to_vk_usage(create_info.flags);
    _array_layer_count = (create_info.type == TextureType::TextureCube ||
                          create_info.type == TextureType::TextureCubeArray) ? 6 : 1;

    create_image_view();
}

VulkanTexture::~VulkanTexture() {
    auto device = _context->get_device();
    if (_vk_image_view != VK_NULL_HANDLE) {
        vkDestroyImageView(device->get_device(), _vk_image_view, nullptr);
    }
    if (_owns_image && _vk_image && _vma_alloc) {
        vmaDestroyImage(device->get_vma_allocator(), _vk_image, _vma_alloc);
    }
}

void VulkanTexture::set_layout(VkCommandBuffer cmd_buf,
                               VkImageLayout old_layout, VkImageLayout new_layout,
                               VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = _vk_image;

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    barrier.subresourceRange = range;

    switch (old_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (new_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;
        default:
            break;
    }

    vkCmdPipelineBarrier(cmd_buf, src_stages, dst_stages, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanTexture::transition_image_layout_internal(
        VkFormat fmt, VkImageLayout old_layout, VkImageLayout new_layout,
        std::shared_ptr<zr::core::CommandBuffer> cmd_buf,
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

    VkPipelineStageFlags src_pipe_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dst_pipe_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_pipe_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_pipe_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        bool has_stencil = (fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT);
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (has_stencil) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        src_pipe_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_pipe_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_pipe_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_pipe_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        src_pipe_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_pipe_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_pipe_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_pipe_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(cmd_buf->get(), src_pipe_stage, dst_pipe_stage, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanTexture::copy_buffer_to_image(
        VkBuffer buf, std::shared_ptr<zr::core::CommandBuffer> cmd_buf,
        std::vector<VkBufferImageCopy> regions) {
    vkCmdCopyBufferToImage(cmd_buf->get(), buf, _vk_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           regions.size(), regions.data());
}

VkBufferImageCopy VulkanTexture::create_buffer_image_copy(
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
    region.imageExtent.width  = (_extent.width >> mip_level);
    region.imageExtent.height = (_extent.height >> mip_level);
    uint16_t depth = (_extent.depth >> mip_level) ? (_extent.depth >> mip_level) : 1;
    region.imageExtent.depth = depth;
    return region;
}

void VulkanTexture::update_data(unsigned char* data, uint32_t size,
                                uint8_t base_layer, uint8_t mip_level, 
                                bool generated_mip_map) {
    auto device = _context->get_device();
    auto fmt = get_vk_format();
    
    BufferCreateInfo ci;
    ci.size = size;
    ci.usage = rhi::BufferUsageFlagBit::CopySrc | 0;
    auto stage_buf = std::make_shared<VulkanBuffer>(_context, ci);
    stage_buf->update(data, size);
    
    struct BlitParams{
        uint16_t base_layer;
        uint16_t mip_level;
        uint16_t width;
        uint16_t height;
        uint16_t depth;
    };

    auto blit = [this](VkImage src_vk_image,
                 const BlitParams& src_blit_params, 
                 const BlitParams& dst_blit_params,
                 std::shared_ptr<zr::core::CommandBuffer> cmd_buf) {
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

        transition_image_layout_internal(get_vk_format(), VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, dst_blit_params.mip_level, 1,
                                     vk_image_blit.dstSubresource.baseArrayLayer, 1);
        vkCmdBlitImage(cmd_buf->get(), _vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    _vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_image_blit, VK_FILTER_LINEAR);
        transition_image_layout_internal(get_vk_format(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd_buf, dst_blit_params.mip_level, 1,
                                     vk_image_blit.dstSubresource.baseArrayLayer, 1);
    };

    int width = _create_info.width;
    int height = _create_info.height;

    int num = 0;
    if (generated_mip_map) {
        num = _create_info.mip_num - mip_level - 1;
    }

    auto fn = [&](std::shared_ptr<zr::core::CommandBuffer> cmd_buf) {
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, mip_level, 1, base_layer, 1);
        auto region = create_buffer_image_copy(mip_level, base_layer);
        copy_buffer_to_image(stage_buf->get(), cmd_buf, {region});
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buf, mip_level, 1, base_layer, 1);

        for (int i = 0; i < num; ++i) {
            int src_idx = i + mip_level;
            int dst_idx = src_idx + 1;
            BlitParams src_blit_params{base_layer, (uint16_t)(src_idx), (uint16_t)(width >> (src_idx)), (uint16_t)(height >> (src_idx)), 1};
            BlitParams dst_blit_params{base_layer, (uint16_t)(dst_idx), (uint16_t)(width >> (dst_idx)), (uint16_t)(height >> (dst_idx)), 1};
            blit(_vk_image, src_blit_params, dst_blit_params, cmd_buf);
        }
    };
    for (int i = 0; i < num; ++i) {
        device->get_cmd_pool()->execute_single_cmd(fn);
    }
    
}

void VulkanTexture::transition_image_layout(VkFormat fmt,
                                            VkImageLayout old_layout, VkImageLayout new_layout,
                                            uint8_t base_mip_level, uint8_t mip_level_count) {
    auto device = _context->get_device();
    auto fn = [&](std::shared_ptr<zr::core::CommandBuffer> cmd_buf) {
        transition_image_layout_internal(fmt, old_layout, new_layout,
                cmd_buf, base_mip_level, mip_level_count, 0, 1);
    };
    device->get_cmd_pool()->execute_single_cmd(fn);
}

} // namespace vulkan
} // namespace rhi

// --- VulkanSampleState ---

namespace rhi {
namespace vulkan {

VkFilter VulkanSampleState::filter_type_to_vk(SamplerFilterType type) {
    switch (type) {
        case SamplerFilterType::SF_NEAREST:             return VK_FILTER_NEAREST;
        case SamplerFilterType::SF_LINEAR:              return VK_FILTER_LINEAR;
        case SamplerFilterType::SF_ANISOTROPIC_NEAREST: return VK_FILTER_NEAREST;
        case SamplerFilterType::SF_ANISOTROPIC_LINEAR:  return VK_FILTER_LINEAR;
        default:                                        return VK_FILTER_LINEAR;
    }
}

VkSamplerAddressMode VulkanSampleState::address_mode_to_vk(SamplerAddressMode mode) {
    switch (mode) {
        case SamplerAddressMode::SAM_REPEAT:        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::SAM_MIRROR_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerAddressMode::SAM_CLAMP_EDGE:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::SAM_CLAMP_BORDER:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:                                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkCompareOp VulkanSampleState::compare_function_to_vk(SamplerCompareFunction scf) {
    switch (scf) {
        case SamplerCompareFunction::SCF_NEVER: return VK_COMPARE_OP_NEVER;
        case SamplerCompareFunction::SCF_LESS:  return VK_COMPARE_OP_LESS;
        default:                                return VK_COMPARE_OP_NEVER;
    }
}

VulkanSampleState::VulkanSampleState(std::shared_ptr<zr::RenderContext> context,
                                     const SampleStateCreateInfo& create_info)
    : _context(context) {
    auto device = _context->get_device();

    bool anisotropy = (create_info.mag_filter_type == SamplerFilterType::SF_ANISOTROPIC_NEAREST ||
                       create_info.mag_filter_type == SamplerFilterType::SF_ANISOTROPIC_LINEAR || 
                       create_info.min_filter_type == SamplerFilterType::SF_ANISOTROPIC_NEAREST ||
                       create_info.min_filter_type == SamplerFilterType::SF_ANISOTROPIC_LINEAR
);

    VkSamplerCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    ci.magFilter = filter_type_to_vk(create_info.mag_filter_type);
    ci.minFilter = filter_type_to_vk(create_info.min_filter_type);
    ci.addressModeU = address_mode_to_vk(create_info.address_u);
    ci.addressModeV = address_mode_to_vk(create_info.address_v);
    ci.addressModeW = address_mode_to_vk(create_info.address_w);
    ci.mipLodBias = create_info.mip_bias;
    ci.anisotropyEnable = anisotropy ? VK_TRUE : VK_FALSE;
    ci.maxAnisotropy = anisotropy ? static_cast<float>(create_info.max_anisotropy) : 1.0f;
    ci.compareEnable = (create_info.scf != SamplerCompareFunction::SCF_NEVER) ? VK_TRUE : VK_FALSE;
    ci.compareOp = compare_function_to_vk(create_info.scf);
    ci.minLod = create_info.min_mip_level;
    ci.maxLod = create_info.max_mip_level;
    ci.borderColor = (create_info.border_color == 0) ? VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK
                                                      : VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    ci.unnormalizedCoordinates = VK_FALSE;

    ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    
    vkCreateSampler(device->get_device(), &ci, nullptr, &_vk_sampler);
}

VulkanSampleState::~VulkanSampleState() {
    if (_vk_sampler != VK_NULL_HANDLE) {
        auto device = _context->get_device();
        vkDestroySampler(device->get_device(), _vk_sampler, nullptr);
    }
}

} // namespace vulkan
} // namespace rhi
