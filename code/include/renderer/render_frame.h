//
// Created by zhida.ji1 on 2022/8/15.
//
#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include <core/image.h>
#include "core/descriptor.h"
#include "core/command_buffer.h"
#include "core/buffer.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace zr{
    namespace core{
        class Framebuffer;
    }

    class RenderFrame{
    public:
        virtual ~RenderFrame();
        inline void set_cmd_buf(std::shared_ptr<core::CommandBuffer> cmd_buf) { _cmd_buf = cmd_buf;};
        inline std::shared_ptr<core::CommandBuffer> get_cmd_buf() { return _cmd_buf;};
        inline void set_uniform_buffer(std::shared_ptr<core::Buffer> uniform_buffer) { _uniform_buffer = uniform_buffer;};
        inline std::shared_ptr<core::Buffer> get_uniform_buffer() { return _uniform_buffer;};
        inline std::shared_ptr<core::ImageView> get_display_view() { return _display_view;};
        inline void set_display_view(std::shared_ptr<core::ImageView> display_view) { _display_view = display_view;};
        inline void set_framebuffer(std::shared_ptr<core::Framebuffer> framebuffer) {_framebuffer = framebuffer;};
        inline std::shared_ptr<core::Framebuffer> get_framebuffer() { return _framebuffer;};
        inline void set_display_image(std::shared_ptr<core::Image> display_image) {_display_image = display_image;};
        inline std::shared_ptr<core::Image> get_display_image() { return _display_image;};
    private:
        std::shared_ptr<core::CommandBuffer> _cmd_buf;
        std::shared_ptr<core::Buffer> _uniform_buffer;
        std::shared_ptr<core::ImageView> _display_view;
        std::shared_ptr<core::Image> _display_image;
        std::shared_ptr<core::Framebuffer> _framebuffer;
        // std::shared_ptr<core::Buffer> _ubo;
        // std::shared_ptr<core::ImageView> _depth_view;
        // std::shared_ptr<core::ImageView> _msaa_view;
    };
}
