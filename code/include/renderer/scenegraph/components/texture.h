//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once
#include <memory>
#include <scenegraph/component.h>
#include <vector>

#include <rhi/rhi_resource.h>
#include <rhi/rhi.h>
#include <vulkan/vulkan.h>

namespace zr{
    namespace core{
        class Image;
        class ImageView;
        class Sampler;
        class Device;
    };
    namespace sg{

        enum TextureType {
            TEXTURE_TYPE_NONE = 0,
            TEXTURE_TYPE_DIFFUSE = 1,
            TEXTURE_TYPE_NORMAL = 2,
            TEXTURE_TYPE_MAX
        };

        enum TextureSamplerType {
            // TEXTURE_SAMPLER_TYPE_NONE = 0;
            TEXTURE_SAMPLER_1D = 0,
            TEXTURE_SAMPLER_2D = 1,
            TEXTURE_SAMPLER_3D = 2,
            TEXTURE_SAMPLER_CUBE = 3,
            TEXTURE_SAMPLER_1D_ARRAY = 4,
            TEXTURE_SAMPLER_2D_ARRAY = 5,
            TEXTURE_SAMPLER_CUBE_ARRAY = 6
        };

        class Texture {
        protected:

            struct ImageDataInfo {
                int width;
                int height;
                int depth;
                int com;
                rhi::ColorFormat fmt;
                unsigned char* ptr;
                uint32_t data_len;
                uint32_t layer_idx;
                uint32_t mipmap_level;
                bool reused;
                bool has_upload;
            };

            struct TextureContent{
                // std::shared_ptr<core::ImageView> image_view;
                // std::shared_ptr<core::Image> image;
                // std::shared_ptr<core::Sampler> sampler;

                rhi::TextureRef rhi_texture;
                rhi::SampleStateRef rhi_sampler;

                std::vector<ImageDataInfo> img_data_infos;
                uint32_t mipmap_level_count;
                uint32_t layer_count;
                bool has_upload;
                // TextureSamplerType st;
                // VkFormat fmt;
                rhi::ColorFormat fmt;
                rhi::TextureType st;
            };
        public:
            
            virtual void upload_data(std::shared_ptr<core::Device> device);

            
            virtual rhi::TextureRef get_rhi_texture();
            virtual rhi::SampleStateRef get_rhi_sampler(); 
            virtual ~Texture();

        protected:
            void add_content(const std::string& path, uint16_t width, uint16_t height, uint16_t depth, rhi::ColorFormat fmt,
                rhi::TextureType st, uint8_t mipmap_level_count = 1);
            void add_content(const std::string& path, rhi::TextureType st, uint8_t mipmap_level_count = 1, rhi::ColorFormat fmt = rhi::ColorFormat::R8G8B8A8_SRGB);
            void add_content(const std::vector<std::string>& mipmap_img_paths, rhi::TextureType st, rhi::ColorFormat fmt = rhi::ColorFormat::R8G8B8A8_SRGB);
            void add_hdr_content(const std::string& path, rhi::ColorFormat pixexl_fmt, rhi::ColorFormat tex_fmt, uint8_t mipmap_level_count = 1);
            void upload_data_internal();
            
            void update_2d_data(const TextureContent& tc, 
                                rhi::TextureRef rhi_texture, uint8_t img_data_idx,
                                bool auto_blit);
            void update_cube_data(const TextureContent& tc, 
                                  rhi::TextureRef rhi_texture, uint8_t img_data_idx,
                                  bool auto_blit);
            void get_cube_face_offset(uint16_t size, uint8_t face, uint16_t& row, uint16_t& col) const;
        protected:
            std::vector<TextureContent> _tex_contents;
            uint32_t _active_idx;
        };

        class SingleLayerTexture : public Texture {
        public:
            // for 3d texture
            SingleLayerTexture(const std::string& img_path, uint16_t width, uint16_t height, uint16_t depth, rhi::ColorFormat fmt,
                rhi::TextureType st = rhi::TextureType::Texture2D, uint8_t mipmap_level_count = 1);

            // for common texture
            SingleLayerTexture(const std::string& img_path,
                rhi::TextureType st = rhi::TextureType::Texture2D,
                                uint8_t mipmap_level_count = 1);
            // for multi source mipmap texture
            SingleLayerTexture(const std::vector<std::string>& mipmap_img_paths, 
                rhi::TextureType st = rhi::TextureType::Texture2D);

            SingleLayerTexture(int width, int height, int depth, rhi::ColorFormat fmt, rhi::TextureType st,
                    uint32_t layer_count, uint32_t mipmap_level_count, 
                    rhi::SampleStateRef sp, bool reused);

            void update_content(const unsigned char* data, uint32_t data_len, 
                            uint32_t layer, uint32_t mipmap_level, uint8_t com);        
        };

        class MultiLayerTexture : public Texture {
        public:
            MultiLayerTexture(const std::string& dir_path, const std::string& name_template,
                              uint32_t count, uint8_t mipmap_level_count = 1);
            void set_active_idx(uint32_t idx);
        };
    }
}




