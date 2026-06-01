#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include <memory>

#include <rhi/rhi_definitions.h>
#include <rhi/rhi_resource.h>
#include <rhi/rhi.h>
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

        struct PassResources {
            std::vector<rhi::TextureRef> input_textures;
            rhi::RenderTargetRef render_target;
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
            void add_depth_stencil_output(const std::string& name, const rhi::AttachmentInfo& attachment_info);
            bool add_depth_stencil_input(const std::string& name);
            void add_reslove_output(const std::string& name, const rhi::AttachmentInfo& attachment_info);
            bool add_color_input(const std::string& name);
            void add_color_output(const std::string& name, const rhi::AttachmentInfo& attachment_info);
            void add_preserve(const std::string& name);
            bool add_texture_sample(const std::string& name);
            void prepare();
            void execute(const PassResources& res);
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
        };
    }
}
