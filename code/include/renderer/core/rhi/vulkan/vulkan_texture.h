#pragma once
#include "rhi/rhi_resource.h"
#include <render_context.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>

namespace rhi {
namespace vulkan {

class VulkanTexture : public rhi::Texture {
public:
    VulkanTexture(std::shared_ptr<zr::RenderContext> context,
                  const TextureCreateInfo& create_info);

    VulkanTexture(std::shared_ptr<zr::RenderContext> context,
                  const TextureCreateInfo& create_info,
                  VkImage existing_image);

    virtual ~VulkanTexture() ;

    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;

    // Accessors
    inline VkImage get_image() const { return _vk_image; }
    inline VkImageView get_image_view() const { return _vk_image_view; }
    inline VkFormat get_vk_format() const { return _vk_format; }
    inline VkExtent3D get_extent() const { return _extent; }
    inline VkImageUsageFlags get_vk_usage() const { return _vk_usage; }
    inline VkSampleCountFlagBits get_sample_count() const { return _sample_count; }
    inline VkImageSubresource get_subresource() const { return _subresource; }
    inline uint32_t get_array_layer_count() const { return _array_layer_count; }

    void set_layout(VkCommandBuffer cmd_buf,
                    VkImageLayout old_layout, VkImageLayout new_layout,
                    VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages);

    void update_data(unsigned char* data, uint32_t size, uint8_t base_layer,
                     uint8_t mip_level, bool generated_mip_map);

    void transition_image_layout(VkFormat fmt,
                                 VkImageLayout old_layout, VkImageLayout new_layout,
                                 uint8_t base_mip_level = 0, uint8_t mip_level_count = 1);

private:
    void create_image();
    void create_image_view();
    VkFormat color_format_to_vk(ColorFormat fmt) const;
    VkImageType texture_type_to_vk_image_type(TextureType type) const;
    VkImageViewType texture_type_to_vk_image_view_type(TextureType type) const;
    VkImageUsageFlags texture_flags_to_vk_usage(TextureCreateFlags flags) const;

    void transition_image_layout_internal(VkFormat fmt,
                                          VkImageLayout old_layout, VkImageLayout new_layout,
                                          std::shared_ptr<zr::core::CommandBuffer> cmd_buf,
                                          uint16_t base_mip_level, uint16_t mip_level_count,
                                          uint8_t base_layer, uint16_t layer_count);

    void copy_buffer_to_image(VkBuffer buf, std::shared_ptr<zr::core::CommandBuffer> cmd_buf,
                              std::vector<VkBufferImageCopy> regions);

    VkBufferImageCopy create_buffer_image_copy(uint8_t mip_level, uint8_t base_layer) const;

private:
    std::shared_ptr<zr::RenderContext> _context;
    VkImage _vk_image{VK_NULL_HANDLE};
    VkImageView _vk_image_view{VK_NULL_HANDLE};
    VmaAllocation _vma_alloc{VK_NULL_HANDLE};
    bool _owns_image{true};

    VkExtent3D _extent{};
    VkFormat _vk_format{};
    VkImageUsageFlags _vk_usage{};
    VkSampleCountFlagBits _sample_count{VK_SAMPLE_COUNT_1_BIT};
    VkImageTiling _tiling{VK_IMAGE_TILING_OPTIMAL};
    VkImageSubresource _subresource{};
    uint32_t _array_layer_count{1};
    VkImageType _img_type{VK_IMAGE_TYPE_2D};
};

class VulkanSampleState : public rhi::SampleState {
public:
    VulkanSampleState(std::shared_ptr<zr::RenderContext> context,
                      const SampleStateCreateInfo& create_info);

    ~VulkanSampleState();

    VulkanSampleState(const VulkanSampleState&) = delete;
    VulkanSampleState& operator=(const VulkanSampleState&) = delete;

    inline VkSampler get_vk_sampler() const { return _vk_sampler; }

private:
    static VkFilter filter_type_to_vk(SamplerFilterType type);
    static VkSamplerAddressMode address_mode_to_vk(SamplerAddressMode mode);
    static VkCompareOp compare_function_to_vk(SamplerCompareFunction scf);

private:
    std::shared_ptr<zr::RenderContext> _context;
    VkSampler _vk_sampler{VK_NULL_HANDLE};
};

} // namespace vulkan
} // namespace rhi
