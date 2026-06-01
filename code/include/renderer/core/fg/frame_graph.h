#pragma once
#include <string>
#include <memory>

#include <unordered_map>
#include <unordered_set>
#include <core/fg/fg_render_resource.h>
#include <core/fg/fg_render_pass.h>
namespace zr {
    namespace core {
        class Device;
        class Swapchain;
        class Image;
        class ImageView;
        class RenderPass;
        class Framebuffer;
        class CommandBuffer;
        class CommandPool;
        class 
        FrameGraph {

            public:
            FrameGraph() : _need_bake(true){

            };
            virtual ~FrameGraph();
            void reset();
            std::shared_ptr<FgRenderPass> add_pass(const std::string& pass_name, bool is_swapchain = false);
            
            void bake();

            std::shared_ptr<FgRenderTextureResource> get_tex_res(const std::string& name);
            bool contains_tex_res(const std::string& name) const;

            void execute();

            inline bool need_bake() const {
                return _need_bake;
            }

            void add_image_view(const std::string& tex_name, std::shared_ptr<ImageView> img_view);

            std::shared_ptr<rhi::Texture> get_image(const std::string& img_name, uint8_t idx = 0);

            protected:

            void order_passes();

            void compute_texture_lifetimes();

            void create_images(uint8_t swapchain_num);

            private:
            bool _need_bake;
            
            std::unordered_map<std::string, std::shared_ptr<FgRenderTextureResource>> _tex_res_map;
            std::unordered_map<std::string, std::vector<std::shared_ptr<rhi::Texture>>> _imgs_map;
            std::unordered_map<std::string, std::shared_ptr<FgRenderPass>> _render_pass_map;
            std::string _swapchain_pass_name;
      
            uint16_t _active_frame_idx = 0; 

            std::vector<std::string> _ordered_passes;

            std::unordered_map<std::string, rhi::TextureRef> _tex_handle_map;
        };
    };
};
