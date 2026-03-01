#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include <memory>
namespace zr {

    namespace sg {
        class Scene;
    };
    namespace core {
        class FgRenderTextureResource;
        class FrameGraph;
        class RendererInterface;
        class PipelineManager;
        class CommandBuffer;
        class ImageView;
        class Device;
        class RenderPass;
        class Swapchain;
        class Image;
        class Framebuffer;
        class FgRenderPassGroup;

        enum AttachmentType{
            ATTACHMENT_TYPE_COLOR = 0,
            ATTACHMENT_TYPE_DEPTH = 1,
            ATTACHMENT_TYPE_RESLOVE = 2,
            ATTACHMENT_TYPE_INPUT = 3,
            ATTACHMENT_TYPE_PRESERVE = 4
        };

        class AttachmentInfo {
            public:
            AttachmentInfo();
            uint32_t width;
            uint32_t height;
            uint32_t depth;
            VkFormat fmt;
            VkSampleCountFlagBits samples;
            VkAttachmentLoadOp load_op;
            VkAttachmentStoreOp store_op;
            uint16_t level;
            uint16_t layer;
            VkImageUsageFlags img_usage;
            VkClearValue clear_val;
            std::string img_name;

            // 对应的image是否在不同的frame buffer 中复用，如果复用则不同的frame buffer 中的image view 指向
            // 相同的image，反之则image 与image view一一对应
            bool is_reused;
        };

        class FgRenderpassSetupData{
        public:
            sg::Scene** pp_scene;
            std::shared_ptr<RendererInterface> renderer_interface;
        };

        class FgRenderPass {

            public:
            FgRenderPass() : _fg{nullptr}, _name("") {};
            FgRenderPass(FrameGraph* fg, const std::string& pass_name) : _fg(fg), _name(pass_name) {};
            virtual ~FgRenderPass();
            void add_depth_stencil_output(const std::string& name, const AttachmentInfo& attachment_info);
            bool add_depth_stencil_input(const std::string& name);
            void add_reslove_output(const std::string& name, const AttachmentInfo& attachment_info);
            // bool add_reslove_input(const std::string& name);
            bool add_color_input(const std::string& name);
            void add_color_output(const std::string& name, const AttachmentInfo& attachment_info);
            void add_preserve(const std::string& name);
            bool add_texture_sample(const std::string& name);
            void prepare(std::shared_ptr<Device> device);
            void execute(std::shared_ptr<CommandBuffer> cmd_buf, std::shared_ptr<Device> device);
            std::unordered_set<std::string> get_outputs() const;
            void set_pipeline_mgr(std::shared_ptr<PipelineManager> mgr);

            inline std::vector<std::shared_ptr<ImageView>> get_color_input_views() {
                return extenal_get_views(_color_inputs);
            }

            inline std::vector<std::shared_ptr<ImageView>> get_depth_stencil_views() {
                return extenal_get_views(_depth_stencil_inputs);
            }

            inline std::vector<std::shared_ptr<ImageView>> get_input_views() {
                return extenal_get_views(_inputs);
            }

            inline std::vector<std::shared_ptr<ImageView>> get_color_output_views() {
                return extenal_get_views(_color_outputs);
            }

            inline const std::vector<std::string>& get_depth_stencil_inputs() const {
                return _depth_stencil_inputs;
            }

            inline const std::vector<std::string>& get_color_inputs() const {
                return _color_inputs;
            }

            inline const std::vector<std::string>& get_inputs() const {
                return _inputs;
            }

            inline const std::vector<std::string>& get_color_outputs() const {
                return _color_outputs;
            }

            inline const std::vector<std::string>& get_resloved_outputs() const {
                return _resloved_outputs;
            }

            inline const std::vector<std::string>& get_depth_stencil_outputs() const {
                return _depth_stencil_outputs;
            }

            inline const std::string& get_name() const {
                return _name;
            }

            inline void set_setup_data(const FgRenderpassSetupData& setup_data) {
                _setup_data = setup_data;
            }

            inline FgRenderpassSetupData& get_setup_data() {
                return _setup_data;
            }

            inline const std::vector<std::string>& get_texture_samples() {
                return _texture_samples;
            }

            inline std::shared_ptr<RenderPass> get_render_pass() const {
                return _render_pass;
            }

            inline void set_render_pass(std::shared_ptr<RenderPass> render_pass) {
                _render_pass = render_pass;
            }

            inline void set_group(std::shared_ptr<FgRenderPassGroup> group) {
                _group = group;
            }

            inline std::shared_ptr<FgRenderPassGroup> get_group() const {
                return _group;
            }

            VkSampleCountFlagBits get_sample_count();

            VkExtent2D get_display_size();

            protected:
            std::vector<std::shared_ptr<ImageView>> extenal_get_views(const std::vector<std::string>& names);

            private:
            FgRenderpassSetupData _setup_data;
            FrameGraph* _fg;
            std::string _name;
            std::vector<std::string> _depth_stencil_inputs;
            std::vector<std::string> _color_inputs;
            std::vector<std::string> _inputs;
            std::vector<std::string> _color_outputs;
            std::vector<std::string> _depth_stencil_outputs;
            std::vector<std::string> _resloved_outputs;
            std::vector<std::string> _texture_samples;
            std::shared_ptr<RenderPass> _render_pass;
            std::shared_ptr<FgRenderPassGroup> _group;
            // std::shared_ptr<PipelineManager>
        };
    
        class FgRenderPassGroup {
            public:
            FgRenderPassGroup(FrameGraph* fg) : _fg (fg) {};

            virtual ~FgRenderPassGroup();

            void bake(std::shared_ptr<Device> device, 
                        std::shared_ptr<Swapchain> swapchain);

            void insert_pass(const std::string& pass_name); 

            bool is_output_res(const std::string& res_name) const;

            bool is_depended(std::shared_ptr<FgRenderPassGroup> group) const;

            void prepare_renderpasses(std::shared_ptr<Device> device);

            void execute(std::shared_ptr<Device> device, uint16_t active_frame_idx, 
                        std::shared_ptr<CommandBuffer> cmd_buf);

            std::shared_ptr<ImageView> get_image_view(const std::string& tex_name);            

            inline bool contains_pass(const std::string& pass_name) const {
                return _sub_passes.find(pass_name) != _sub_passes.end();
            }

            protected:
            void order_pass();
            std::unordered_map<std::string, std::unordered_set<std::string>>  fetch_dependencies() const;
            void create_image_res(std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain);
            void create_image(const std::string& res_name, std::shared_ptr<Device> device, 
                        std::shared_ptr<Swapchain> swapchain, bool is_swapchain_res = false);
            void create_attachment(const std::string& res_name, std::shared_ptr<Device> device,
                                   std::shared_ptr<Swapchain> swapchain, bool is_swapchain_res = false);    
            void create_subpass(std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain);   
            
            void create_subpass_dependencies(std::shared_ptr<Device> device,
                                             std::shared_ptr<Swapchain> swapchain);

            void create_renderpass(std::shared_ptr<Device> device,
                                   std::shared_ptr<Swapchain> swapchain);

            void create_pipeline_mgrs(std::shared_ptr<Device> device, 
                                        std::shared_ptr<Swapchain> swapchain);   

            void create_framebuffer(std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain);                                        

            private:
            std::unordered_set<std::string> _input_res;
            std::unordered_set<std::string> _output_res;
            std::unordered_set<std::string> _sub_passes;
            std::vector<std::string> _ordered_passes;

            std::vector<std::vector<std::shared_ptr<ImageView>>> _img_views;
            
            std::vector<VkClearValue> _clear_vals;
            std::vector<VkAttachmentDescription> _attachment_descs;

            std::unordered_map<std::string, uint16_t> _tex_index_map;
            std::vector<std::shared_ptr<Image>> _imgs;

            std::vector<std::vector<VkAttachmentReference>> _attachment_refs;

            std::vector<std::vector<uint32_t>> _preserved_attachments_arr;

            std::vector<VkSubpassDescription> _subpass_descs;

            std::vector<VkSubpassDependency> _subpass_dependecies;

            std::shared_ptr<RenderPass> _render_pass;

            std::vector<std::shared_ptr<Framebuffer>> _frame_buffers;

            FrameGraph* _fg;
        };
    }
}
