//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
namespace zr {
    namespace core {
        class Device;
        class Framebuffer;
        class Swapchain;
        class Image;
        class ImageView;
        class CommandBuffer;
        struct SubpassInfo {
            std::vector<uint32_t> input_attachments;
            std::vector<uint32_t> output_attachments;
            std::vector<uint32_t> color_reslove_attachments;
            bool disable_depth_stencil_attachment;
            uint32_t depth_stencil_reslove_attachment;
            // VkResloveModeFlagBits depth_stencil_reslove_mode;
        };

        class RenderPass {
        public:
            RenderPass(std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain,
                       bool enable_depath_test = true,
                       bool enable_msaa = true,
                       VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT);

            RenderPass(std::shared_ptr<Device> device, VkExtent2D display_size/*std::shared_ptr<Swapchain> swapchain*/,
                       const VkRenderPassCreateInfo& rpc);

        public:
            void begin(std::shared_ptr<CommandBuffer> cmd_buf, 
                        std::shared_ptr<Framebuffer> framebuffer, 
                        std::vector<VkClearValue> clear_vals);
            void end(std::shared_ptr<CommandBuffer> cmd_buf);
            void next(std::shared_ptr<CommandBuffer> cmd_buf);
            void set_render_area(int offset_x, int offset_y, int width, int height);

            virtual ~RenderPass();
            inline VkRenderPass get() const { return _vk_render_pass;};

            inline std::vector<std::shared_ptr<Framebuffer>> get_framebuffers() { return _framebuffers;};

        private:
            void enable_depth(VkSampleCountFlagBits samples, VkFormat format,
                              std::vector<VkAttachmentDescription>& descs, VkSubpassDescription& subpass_desc);
            void create_depth_attachment(VkSampleCountFlagBits samples, VkExtent2D extent);

            void enable_msaa(VkSampleCountFlagBits samples, VkFormat format,
                             std::vector<VkAttachmentDescription>& descs, VkSubpassDescription& subpass_desc);
            void create_msaa_attachment(VkSampleCountFlagBits samples, VkExtent2D extent);

            void enable_color(VkSampleCountFlagBits samples, VkFormat format,
                              std::vector<VkAttachmentDescription>& descs, VkSubpassDescription& subpass_desc);

        private:
            bool _enabled_depth_test{false};
            bool _enabled_msaa{false};
            // size_t _sub_pass_count{0};
            std::shared_ptr<Device> _device{nullptr};
            VkRenderPass _vk_render_pass{};
            std::shared_ptr<Image> _msaa_image{};
            std::shared_ptr<ImageView> _msaa_image_view{};
            std::shared_ptr<Image>_depth_image{};
            std::shared_ptr<ImageView>_depth_image_view{};
            std::vector<std::shared_ptr<Framebuffer>> _framebuffers;

            VkExtent2D _display_size{};
            VkRect2D _render_area{};
            

            VkAttachmentReference _color_ref{};
            VkAttachmentReference _msaa_ref{};
            VkAttachmentReference _depth_ref{};
        };
    }
}
