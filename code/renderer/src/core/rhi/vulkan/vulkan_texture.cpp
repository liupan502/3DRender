#include <rhi/vulkan/vulkan_texture.h>
#include <core/device.h>
#include <core/vk_common.h>
#include <core/buffer.h>
#include <core/command_pool.h>
#include <core/command_buffer.h>

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
    if (static_cast<uint64_t>(flags) & static_cast<uint64_t>(TextureCreateFlags::RenderTargetable))
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (static_cast<uint64_t>(flags) & static_cast<uint64_t>(TextureCreateFlags::ResolveTargetable))
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (static_cast<uint64_t>(flags) & static_cast<uint64_t>(TextureCreateFlags::DepthStencilTargetable))
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (static_cast<uint64_t>(flags) & static_cast<uint64_t>(TextureCreateFlags::ShaderResource))
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (static_cast<uint64_t>(flags) & static_cast<uint64_t>(TextureCreateFlags::CPUWritable))
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (static_cast<uint64_t>(flags) & static_cast<uint64_t>(TextureCreateFlags::InputAttachmentRead))
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
    _array_layer_count = (create_info.type == TextureType::TextureCube ||
                          create_info.type == TextureType::TextureCubeArray) ? 6 : 1;

    create_image();
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

}

VulkanTexture::~VulkanTexture() {
    if (_owns_image && _vk_image && _vma_alloc) {
        auto device = _context->get_device();
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

void VulkanTexture::update_data(unsigned char* data, uint32_t size, uint8_t base_layer,
                                VkFormat fmt) {
    auto device = _context->get_device();
    zr::core::Buffer stage_buf(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stage_buf.update(data, size);

    auto fn = [&](std::shared_ptr<zr::core::CommandBuffer> cmd_buf) {
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, 0, 1, base_layer, 1);
        auto region = create_buffer_image_copy(0, base_layer);
        copy_buffer_to_image(stage_buf.get(), cmd_buf, {region});
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buf, 0, 1, base_layer, 1);
    };
    device->get_cmd_pool()->execute_single_cmd(fn);
}

void VulkanTexture::update_data(unsigned char* data, uint32_t size,
                                uint8_t mip_level, uint8_t base_layer,
                                VkFormat fmt) {
    auto device = _context->get_device();
    zr::core::Buffer stage_buf(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stage_buf.update(data, size);

    auto fn = [&](std::shared_ptr<zr::core::CommandBuffer> cmd_buf) {
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buf, mip_level, 1, base_layer, 1);
        auto region = create_buffer_image_copy(mip_level, base_layer);
        copy_buffer_to_image(stage_buf.get(), cmd_buf, {region});
        transition_image_layout_internal(fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buf, mip_level, 1, base_layer, 1);
    };
    device->get_cmd_pool()->execute_single_cmd(fn);
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
