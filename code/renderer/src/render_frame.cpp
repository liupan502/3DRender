//
// Created by zhida.ji1 on 2022/8/15.
//
#include "render_frame.h"

using namespace zr;

RenderFrame::~RenderFrame() {
    _cmd_buf = nullptr;
    _uniform_buffer = nullptr;
    _display_view = nullptr;
    _display_image = nullptr;
    _framebuffer = nullptr;
}
