#include <avm_renderer.h>
#include <core/swapchain.h>
#include <core/fg/frame_graph.h>
#include <render_context.h>
#include <core/device.h>
#include <core/command_buffer.h>
#include <core/vk_common.h>

using namespace zr;

void AvmRenderer::add_main_pass() {
    std::shared_ptr<core::FgRenderPass> main_pass = _fg->add_pass("main_pass", true);
    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    _video_renderer = std::make_shared<core::AvmVideoRenderer>();

    VkExtent3D extent;
    extent.width = _img_size.x;
    extent.height = _img_size.y;
    extent.depth = 1;

    auto surround_tex = std::make_shared<sg::SingleLayerTexture>(extent, VK_FORMAT_R8G8B8A8_UNORM,
                sg::TEXTURE_SAMPLER_2D_ARRAY, 4, 1, nullptr, true);
    _texs.emplace_back(surround_tex);
    _video_renderer->set_surround_tex(surround_tex);

    set_up.renderer_interface = _video_renderer;
    main_pass->set_setup_data(set_up);

    std::shared_ptr<core::Swapchain> sc = _context->get_swapchain();

    core::AttachmentInfo color_output_info{};
    color_output_info.fmt = sc->get_suitable_format();
    color_output_info.depth = 1;
    color_output_info.width = sc->get_display_size().width;
    color_output_info.height = sc->get_display_size().height;
    color_output_info.img_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    color_output_info.layer = 0;
    color_output_info.level = 0;
    color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
    color_output_info.clear_val = VkClearValue {
        .color = {1.0f, 1.0f, 1.0f, 1.0f}
    };
    color_output_info.is_reused = false;
    color_output_info.img_name = "main_pass_color_output_img";
    color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;

    std::string pass_name = "main_pass_color_output";
    main_pass->add_color_output(pass_name, color_output_info);
}

bool AvmRenderer::init_internel(VkSampleCountFlagBits sample_count) {
    _sample_count = sample_count;
    _fg = std::make_shared<core::FrameGraph>();
    add_main_pass();

    if (_fg->need_bake()) {
        _fg->bake(_context->get_device(),
                  _context->get_swapchain(), _context->get_cmd_pool());
    }
    return true;
}

void AvmRenderer::set_avm_mode(core::AvmMode mode) {
    _mode = mode;
    if (_video_renderer) {
        _video_renderer->set_mode(mode);
    }
}

void AvmRenderer::update_texture(core::AvmMode mode, const unsigned char* data,
     uint32_t data_len, uint8_t com) {

    uint8_t tex_idx = 0;
    uint8_t layer_idx = 0;

    switch (mode) {
    case core::AVM_MODE_SOURROUND_LEFT : {
        tex_idx = 0;
        layer_idx = 0;
        break;
    }
    case core::AVM_MODE_SOURROUND_RIGHT : {
        tex_idx = 0;
        layer_idx = 1;
        break;
    }
    case core::AVM_MODE_SOURROUND_REAR : {
        tex_idx = 0;
        layer_idx = 2;
        break;
    }
    case core::AVM_MODE_SOURROUND_FRONT : {
        tex_idx = 0;
        layer_idx = 3;
        break;
    }
    default:
        break;
    }

    _texs[0]->update_content(data, data_len, layer_idx, 0, com);

}

void AvmRenderer::render_scene(std::shared_ptr<sg::Scene> scene) {
    upload_texs(_context->get_device());

    // Acquire image using RenderContext
    uint32_t active_frame_idx = _context->acquire_image(_context->get_semaphore());
    if (active_frame_idx == UINT32_MAX) {
        return;
    }

    _fg->execute(active_frame_idx, _context->get_device());

    // Submit and present using RenderContext
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    _context->submit(
        _context->get_queue()->get(),
        _fg->get_command_buf(active_frame_idx)->get(),
        _context->get_semaphore(),
        wait_stage,
        _context->get_fence()
    );

    _context->present(_context->get_queue()->get(), active_frame_idx);

    ++_frame_id;
}

void AvmRenderer::upload_texs(std::shared_ptr<core::Device> device) {
    for (auto tex : _texs) {
        tex->upload_data(device);
    }
}

void AvmRenderer::set_img_size(const glm::vec2& size) {
    _img_size = size;
}

void AvmRenderer::set_img_offset(const glm::vec2& offset) {
    if (_video_renderer) {
        _video_renderer->set_img_offset(offset);
    }
}

// 400
void AvmRenderer::set_focal(float focal) {
    if (_video_renderer) {
        _video_renderer->set_focal(focal);
    }
}
