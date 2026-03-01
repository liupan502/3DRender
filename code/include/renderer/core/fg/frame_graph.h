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

            friend class FgRenderPassGroup;

            public:
            FrameGraph() : _need_bake(true){

            };
            virtual ~FrameGraph();
            void reset();
            std::shared_ptr<FgRenderPass> add_pass(const std::string& pass_name, bool is_swapchain = false);
            
            void bake(std::shared_ptr<Device> device, 
                        std::shared_ptr<Swapchain> swapchain, 
                        std::shared_ptr<CommandPool> cmd_pool);

            std::shared_ptr<FgRenderTextureResource> get_tex_res(const std::string& name);
            bool contains_tex_res(const std::string& name) const;

            void execute(uint16_t active_frame_idx, std::shared_ptr<Device> device);

            inline std::shared_ptr<CommandBuffer> get_command_buf(uint16_t active_idx) {
                return _cmd_bufs[active_idx];
            }

            inline bool need_bake() const {
                return _need_bake;
            }

            void add_image_view(const std::string& tex_name, std::shared_ptr<ImageView> img_view);

            // void add_image(const std::string& img_name, std::shared_ptr<Image> img);

            // bool has_image(const std::string& img_name);

            std::shared_ptr<Image> get_image(const std::string& img_name, uint8_t idx = 0);

            std::shared_ptr<ImageView> get_image_view(const std::string& tex_name);

            void set_viewport(VkViewport viewport);    
            protected:

            void seperate_render_pass();

            void order_group();

            void create_cmd_bufs(std::shared_ptr<Device> device, 
                                    std::shared_ptr<Swapchain> swapchain,
                                    std::shared_ptr<CommandPool> cmd_pool); 

            void create_images(std::shared_ptr<Device> device, uint8_t swapchain_num);     
                   


            private:
            bool _need_bake;
            
            std::unordered_map<std::string, std::shared_ptr<FgRenderTextureResource>> _tex_res_map;
            std::unordered_map<std::string, std::vector<std::shared_ptr<Image>>> _imgs_map;
            std::unordered_map<std::string, std::shared_ptr<FgRenderPass>> _render_pass_map;
            std::string _swapchain_pass_name;
      
            std::vector<std::shared_ptr<CommandBuffer>> _cmd_bufs;

            uint16_t _active_frame_idx = 0; 

            std::vector<std::shared_ptr<FgRenderPassGroup>> _groups;

            std::unordered_map<std::string, std::vector<std::shared_ptr<ImageView>>> _img_views_map;
        };
    };
};
