//
// Created by zhida.ji1 on 2022/8/16.
//
#pragma once


#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <scenegraph/scene_graph.h>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#include <android/native_window.h>
#elif  PLATFORM_GLFW
#include <GLFW/glfw3.h>
#endif
namespace zr{
    namespace core{
        class Device;
        class RenderPass;
        class Pipeline;
        class BasePipeline;
        class Image;
        class ImageView;
        class PipelineManager;
    };

    namespace sg {
        class Scene;
    }

    class RenderFrame;
    class RenderTarget;
    class RenderContext;
    class Renderer{
    public:
        Renderer() = default;
        virtual ~Renderer();

#ifdef PLATFORM_ANDROID
        bool init(AAssetManager* asset_mgr, ANativeWindow* window,
                  VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_8_BIT);
#elif  PLATFORM_GLFW
        bool init(GLFWwindow* window,
                  VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_8_BIT);
#endif
        bool init_internel(VkSampleCountFlagBits sample_count);
        // Device* device();
        void render_scene(std::shared_ptr<sg::Scene> scene);

        void set_viewport(int offset_x, int offset_y, uint16_t width, uint16_t height);

    protected:
        bool begin_frame() ;
        void end_frame() ;
        void render_mesh(std::shared_ptr<sg::Mesh> mesh);
        bool init_sync_res();



    private:
        std::vector<std::shared_ptr<RenderFrame>> _frames;
        std::vector<std::shared_ptr<RenderTarget>> _targets;
        std::shared_ptr<RenderContext> _context;
        std::shared_ptr<core::RenderPass> _render_pass;
        std::shared_ptr<core::Pipeline> _current_graphic_pipeline;
        std::shared_ptr<core::PipelineManager> _pipeline_manager;

        VkViewport _viewport;

        uint32_t _active_frame{0};
        VkSemaphore _vk_semaphore{VK_NULL_HANDLE};
        VkFence _vk_fence{VK_NULL_HANDLE};
    };
}
