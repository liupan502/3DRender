#pragma once
#include <vulkan/vulkan.h>
#include <unordered_set>
#include <unordered_set>
#include <string>
#include <memory>
#include <core/fg/fg_render_pass.h>
namespace zr{
    namespace core{
        

        struct ResourceDimensions
        {
            VkFormat fmt = VK_FORMAT_UNDEFINED;
            uint16_t width = 0;
            uint16_t height = 0;
            uint16_t depth = 1;
            uint16_t layers = 1;
            uint16_t levels = 1;
            uint16_t samples = 1;
            VkImageUsageFlags img_usage = 0;
        };

        enum RenderResourceType {
            RENDER_RESOURCE_TYPE_BUFFER = 0,
            RENDER_RESOURCE_TYPE_TEXTURE = 1,
            RENDER_RESOURCE_TYPE_MAX
        };
        
        class FgRenderResource {
            public:
            FgRenderResource(const FgRenderResource& other) = default;
            FgRenderResource(const std::string& name  = "") : _name(name), _type(RENDER_RESOURCE_TYPE_MAX) {};

            inline void add_read_pass(const std::string& pass_name) {
                _read_pass_names.insert(pass_name);
            }

            void test() const {

            };

            inline const std::unordered_set<std::string>& get_read_passes() const {
                return _read_pass_names;
            } 

            inline void set_write_pass(const std::string& pass_name) {
                _wirte_pass_name = pass_name;
            }

            inline const std::string& get_write_pass() const {
                return _wirte_pass_name;
            }

            
            inline const std::string& get_name() const {
                return _name;
            }

            inline RenderResourceType get_type() const {
                return _type;
            }

            protected:
            std::string _name;
            RenderResourceType _type;
            std::string _wirte_pass_name;
            std::unordered_set<std::string> _read_pass_names;


        };

        class FgRenderBufferResource : public FgRenderResource {
            
        };

        class FgRenderTextureResource : public FgRenderResource {
            public:
            FgRenderTextureResource(const std::string& name = "");
            inline void add_img_usage(VkImageUsageFlags new_img_usage) {
                _img_usage |= new_img_usage;
                _attachment_info.img_usage = _img_usage;
            }
            inline VkImageUsageFlags get_img_usage() const {
                return _img_usage;
            }

            inline void set_attachment_info(const AttachmentInfo& attachment_info) { 
                _attachment_info = attachment_info;
                add_img_usage(_attachment_info.img_usage);
                _attachment_info.img_usage = _img_usage;
            };

            inline const AttachmentInfo& get_attachment_info() const {
                return _attachment_info;
            }

            
            private:
            
            AttachmentInfo _attachment_info;
            VkImageUsageFlags _img_usage;
        };
    }
}
