//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <vk_mem_alloc.h>

namespace zr {
    namespace core {
        class ImageView;
        class Device;
        class CommandBuffer;
        class Image {
        public:
            struct BlitParams{
                uint16_t base_layer;
                uint16_t mip_level;
                uint16_t width;
                uint16_t height;
                uint16_t depth;
            };

        public:
            Image(std::shared_ptr<Device> device, const VkExtent3D& extent,
                  VkFormat fmt, VkImageUsageFlags img_usage, VkMemoryPropertyFlagBits mem_usage,
                  VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
                  uint32_t mip_levels_count = 1,
                  uint32_t array_layers_count = 1,
                  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                  VkImageCreateFlags flags = 0,
                  VkImageType img_type = VK_IMAGE_TYPE_2D,
                  uint32_t num_queue_families = 0,
                  uint32_t* queue_families = nullptr);

            Image(std::shared_ptr<Device> device, VkImage vk_image) : _device(device), _vk_image(vk_image) {

            };

            virtual ~Image();

            void set_layout(VkCommandBuffer cmd_buf,
                            VkImageLayout old_img_layout, VkImageLayout new_img_layout,
                            VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages);

            inline VkImageType get_type() const { return _type; };
            inline VkExtent3D get_extent() const { return _extent; };
            inline VkFormat get_format() const { return _fmt; };
            inline VkImageUsageFlags get_usage() const { return _usage; };
            inline VkSampleCountFlagBits get_sample_count() const { return _sample_count; };
            inline VkImageTiling get_tiling() const { return _tiling; };
            inline VkImageSubresource get_subresource() const { return _subresource; };
            inline uint32_t get_array_layer_count() const { return _array_layer_count; };
            inline VkImage* get() { return &_vk_image; };
            const VkDevice& get_device() const;

            // 更新image 的数据，如果mipmap level count > 1,会自动生成不同level的图像数据
            void update_data(unsigned char* data, uint32_t size, uint8_t base_layer_count,
                             VkFormat fmt = VK_FORMAT_R8G8B8A8_SRGB);
            // 更新image 的数据，将图像数据更新到mipmap level , base_layer_count对应的view中
            void update_data(unsigned char* data, uint32_t size, uint8_t mip_map_level,
                             uint8_t base_layer_count, VkFormat fmt = VK_FORMAT_R8G8B8A8_SRGB);

            void blit(VkImage src_vk_img,
                      const BlitParams& src_blit_params, const BlitParams& dst_blit_params,
                      std::shared_ptr<CommandBuffer> cmd_buf);

            void transition_image_layout(VkFormat fmt,
                                         VkImageLayout old_layout, VkImageLayout new_layout,
                                         uint8_t base_mip_level = 0, uint8_t mip_level_count = 1);

        protected:

            void transition_image_layout_internal(VkFormat fmt,
                                         VkImageLayout old_layout, VkImageLayout new_layout,
                                         std::shared_ptr<CommandBuffer> cmd_buf,
                                                  uint16_t base_mip_level, uint16_t mip_level_count,
                                                  uint8_t base_layer, uint16_t layer_count);
            void copy_buffer_to_image(VkBuffer buf, std::shared_ptr<CommandBuffer> cmd_buf,
                                      std::vector<VkBufferImageCopy> regions);

            VkBufferImageCopy create_buffer_imgage_copy(uint8_t mip_level, uint8_t base_layer) const;

            void blit_internal(VkImage src_vk_img, VkImageBlit vk_image_blit,
                               std::shared_ptr<CommandBuffer> cmd_buf, uint8_t mip_level);

        protected:
            VkImageType _type{};
            VkExtent3D _extent{};
            VkFormat _fmt{};
            VkImageUsageFlags _usage{};
            VkSampleCountFlagBits _sample_count{};
            VkImageTiling _tiling{};
            VkImageSubresource _subresource{};
            uint32_t _array_layer_count{0};
            VkImageType _img_type;

            // std::vector<ImageView*> _views;
            // void* _mapped_data{nullptr};
            // bool _mapped{false};
            std::shared_ptr<Device> _device{nullptr};
            VkImage _vk_image {VK_NULL_HANDLE};
            VkDeviceMemory _vk_image_memory{VK_NULL_HANDLE};

            VmaAllocation _vma_alloc;

        };
    }
}

