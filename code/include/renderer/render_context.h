//
// Created by zhida.ji1 on 2022/8/15.
//
#pragma once
#include <memory>
#include "renderer.h"

namespace zr{
    namespace core {
        class Instance;
        class PhysicalDevice;
        class Device;
        class Queue;
        class CommandPool;
        class Swapchain;
        class DescriptorPool;
        class PipelineLayout;
        class DescriptorLayout;
    }

    class RenderContext{
    public:
        RenderContext() {};
        virtual ~RenderContext();
#ifdef PLATFORM_ANDROID
        bool init(AAssetManager* asset_mgr, ANativeWindow* window, VkFormat swapchain_fmt);
#elif  PLATFORM_GLFW
        bool init(GLFWwindow* window, VkFormat swapchain_fmt);
#endif
        inline std::shared_ptr<core::Swapchain> get_swapchain() { return _swap_chain;};
        inline std::shared_ptr<core::CommandPool> get_cmd_pool() { return _cmd_pool;};
        inline std::shared_ptr<core::Device> get_device() { return _device;};
        // inline std::shared_ptr<core::DescriptorPool> get_desc_pool() { return _desc_pool;};
        inline std::shared_ptr<core::PhysicalDevice> get_gpu() { return _physical_device;};
        inline std::shared_ptr<core::Queue> get_queue() { return _queue;};
        inline std::shared_ptr<core::Instance> get_instance() { return _instance; };
        // inline std::shared_ptr<core::PipelineLayout> get_pipeline_layout() { return _pipeline_layout;};

        // Swapchain management
        uint32_t acquire_image(VkSemaphore signal_semaphore, uint64_t timeout = UINT64_MAX);
        void present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore = VK_NULL_HANDLE);

        // Command submission
        void submit(VkQueue queue, VkCommandBuffer cmd_buf, VkSemaphore wait_semaphore,
                   VkPipelineStageFlags wait_stage, VkFence fence);

        // Sync objects
        VkSemaphore get_semaphore() const { return _vk_semaphore; }
        VkFence get_fence() const { return _vk_fence; }

        // Frame index tracking
        uint32_t get_current_frame_index() const { return _current_frame_index; }

    private:
        void setup_debug_messenger();
        std::shared_ptr<core::PhysicalDevice> _physical_device;
        std::shared_ptr<core::Instance> _instance;
        std::shared_ptr<core::Device> _device;
        std::shared_ptr<core::CommandPool> _cmd_pool;
        std::shared_ptr<core::Queue> _queue;
        std::shared_ptr<core::Swapchain> _swap_chain;
        // std::shared_ptr<core::DescriptorPool> _desc_pool;
        // std::shared_ptr<core::DescriptorLayout> _desc_layout;
        // std::shared_ptr<core::PipelineLayout> _pipeline_layout;

        // Sync objects
        VkSemaphore _vk_semaphore{VK_NULL_HANDLE};
        VkFence _vk_fence{VK_NULL_HANDLE};

        // Frame tracking
        uint32_t _current_frame_index{0};

        VkDebugUtilsMessengerEXT _debug_messenger{VK_NULL_HANDLE};
    };
}
