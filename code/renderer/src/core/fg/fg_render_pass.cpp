#include <core/fg/fg_render_pass.h>
#include <core/fg/frame_graph.h>
#include <core/pipeline_manager.h>
#include <scenegraph/scene.h>
#include <core/renderers/renderer_interface.h>
#include <core/swapchain.h>
#include <core/command_buffer.h>
#include <core/render_pass.h>
#include <core/image.h>
#include <core/image_view.h>
#include <core/framebuffer.h>
#include <assert.h>
#include <algorithm>

#include <rhi/rhi_definitions.h>
#include <rhi/rhi_resource.h>
using namespace zr::core;

bool FgRenderPass::add_color_input(const std::string& name) {
    if (!_fg->contains_tex_res(name)) {
        return false;
    }
    auto tex_res = _fg->get_tex_res(name);
    _color_inputs.emplace_back(name);
    _inputs.emplace_back(name);
    tex_res->add_read_pass(get_name());
    tex_res->add_img_usage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    return true;
}

void FgRenderPass::add_color_output(const std::string &output_name, const rhi::AttachmentInfo &attachment_info) {
    std::string name = output_name;
    assert(_fg->contains_tex_res(name) == false);
    auto tex = _fg->get_tex_res(name);

    tex->add_img_usage(static_cast<uint64_t>(rhi::TextureCreateFlagBit::RenderTargetable));
    tex->set_attachment_info(attachment_info);
    _color_outputs.emplace_back(name);
    tex->set_write_pass(get_name());
}

FgRenderPass::~FgRenderPass() {
    _render_pass = nullptr;
}

void FgRenderPass::add_reslove_output(const std::string &name, const rhi::AttachmentInfo &attachment_info) {
    assert(_fg->contains_tex_res(name) == false);
    auto tex = _fg->get_tex_res(name);
    _resloved_outputs.emplace_back(name);
    tex->add_img_usage(rhi::TextureCreateFlagBit::RenderTargetable | 0);
    tex->set_attachment_info(attachment_info);
    tex->set_write_pass(get_name());
}

bool FgRenderPass::add_depth_stencil_input(const std::string& name) {
    if (!_fg->contains_tex_res(name)) {
        return false;
    }
    _depth_stencil_inputs.emplace_back(name);
    _inputs.emplace_back(name);
    auto tex_res = _fg->get_tex_res(name);
    tex_res->add_read_pass(get_name());
    tex_res->add_img_usage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    return true;
}

void FgRenderPass::add_depth_stencil_output(const std::string &name, const rhi::AttachmentInfo &attachment_info) {
    assert(_fg->contains_tex_res(name) == false);
    auto tex = _fg->get_tex_res(name);
    tex->add_img_usage(rhi::TextureCreateFlagBit::DepthStencilTargetable | 0);
    tex->set_attachment_info(attachment_info);
    _depth_stencil_outputs.emplace_back(name);
    tex->set_write_pass(get_name());
}

bool FgRenderPass::add_texture_sample(const std::string& name) {
    if (!_fg->contains_tex_res(name)) {
        return false;
    }
    auto tex_res = _fg->get_tex_res(name);
    tex_res->add_img_usage(VK_IMAGE_USAGE_SAMPLED_BIT);
    _texture_samples.emplace_back(name);
    _inputs.emplace_back(name);
    tex_res->add_read_pass(get_name());
    return true;
}

std::unordered_set<std::string> FgRenderPass::get_outputs() const {
    std::unordered_set<std::string> outputs;
    auto &resloved_outputs = get_resloved_outputs();
    outputs.insert(resloved_outputs.begin(), resloved_outputs.end());
    auto &color_outputs = get_color_outputs();
    outputs.insert(color_outputs.begin(), color_outputs.end());
    auto &depth_stencil_outputs = get_depth_stencil_outputs();
    outputs.insert(depth_stencil_outputs.begin(), depth_stencil_outputs.end());
    return outputs;
}

void FgRenderPass::prepare(const PassResources& res) {
    (void)res;
    _setup_data.renderer_interface->prepare_renderpass(*_setup_data.pp_scene, this, res);
}

void FgRenderPass::execute(const PassResources& res) {
    _setup_data.renderer_interface->render_scene(*_setup_data.pp_scene, res);
}

void FgRenderPass::set_pipeline_mgr(std::shared_ptr<PipelineManager> mgr) {
    // mgr->set_pipeline_create(_setup_data.renderer_interface->get_pipeline_creator());
    // _setup_data.renderer_interface->set_pipeline_mgr(mgr);
}

rhi::SampleCount FgRenderPass::get_sample_count() {
    if (_color_outputs.size() > 0) {
        return _fg->get_tex_res(*_color_outputs.begin())->get_attachment_info().samples;
    }
    return rhi::SampleCount::SC_COUNT_1;
}

VkExtent2D FgRenderPass::get_display_size() {
    if (_color_outputs.size() > 0) {
        auto& attach_info =  _fg->get_tex_res(*_color_outputs.begin())->get_attachment_info();
        return VkExtent2D {attach_info.width, attach_info.height};
    }
    auto& attach_info =  _fg->get_tex_res(*_depth_stencil_outputs.begin())->get_attachment_info();
    return VkExtent2D {attach_info.width, attach_info.height};
}

std::vector<rhi::TextureRef> FgRenderPass::get_color_output_textures() {
    std::vector<rhi::TextureRef> textures;
    if (!_fg) return textures;
    for (auto& name : _color_outputs) {
        textures.push_back(_fg->get_texture_handle(name));
    }
    return textures;
}

std::vector<std::shared_ptr<ImageView>> FgRenderPass::extenal_get_views(const std::vector<std::string>& names) {
    std::vector<std::shared_ptr<ImageView>> views;
    if (!_fg) {
        return views;
    }

     for (auto& view_name : names) {
         auto view = _fg->get_image_view(view_name);
        views.emplace_back(view);
    }
    return views;
}
