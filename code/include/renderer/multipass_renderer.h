#pragma once
#include <core/fg/frame_graph.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <render_context.h>
#include <scenegraph/scene_graph.h>
#include <core/renderers/renderer_interface.h>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#include <android/native_window.h>
#elif  PLATFORM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace zr {
    namespace core {
        class LightRenderer;
        class TaaRenderer;
    }
    class MultiPassRenderer {
        public:
            MultiPassRenderer();
            virtual ~MultiPassRenderer();
            #ifdef PLATFORM_ANDROID
            bool init(AAssetManager* asset_mgr, ANativeWindow* window,
                    VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_8_BIT,
                    VkFormat swapchain_fmt = VK_FORMAT_R8G8B8A8_SRGB);
            #elif  PLATFORM_GLFW
            bool init(GLFWwindow* window,
                    VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_8_BIT,
                    VkFormat swapchain_fmt = VK_FORMAT_B8G8R8A8_SRGB);
            #endif
            
            // Device* device();
            virtual void render_scene(std::shared_ptr<sg::Scene> scene);

            void set_viewport(int offset_x, int offset_y, uint16_t width, uint16_t height);

            void wait_idle();

        protected:
            virtual bool init_internel(VkSampleCountFlagBits sample_count);
            bool init_sync_res();

            void prepare_taa();
        protected:
            void add_main_pass();
            void add_taa_pass();
            void add_bloom_pass();
            void add_color_grading_pass();
            void add_output_pass();

            // sky rendering
            void add_transmittance_pass();
            void add_sky_view_pass();

            std::shared_ptr<core::LightRenderer> get_light_render();
            std::shared_ptr<core::TaaRenderer> get_taa_render();
            

        protected:    
            std::shared_ptr<core::FrameGraph> _fg;
            std::shared_ptr<RenderContext> _context;
            uint32_t _active_frame;
            sg::Scene** _scene;
            VkSemaphore _vk_semaphore;
            VkFence _vk_fence;
            VkSampleCountFlagBits _sample_count;
            uint32_t _frame_id{0};

            // taa
            core::CameraInfo _camera_info;

            VkViewport _viewport;

            std::map<std::string, std::vector<std::shared_ptr<core::RendererInterface>>> _render_map;
    };
}
