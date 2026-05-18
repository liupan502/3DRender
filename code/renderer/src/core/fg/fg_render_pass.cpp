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
using namespace zr::core;

AttachmentInfo::AttachmentInfo() {
    width = 0;
    height = 0;
    depth = 1;
    fmt = ColorFormat::None;
    samples = SampleCount::SC_COUNT_1;
    load_op = AttachmentLoadOp::ALO_LOAD;
    store_op = AttachmentStoreOp::ASO_STORE;
    level = 0;
    layer = 0;
    img_usage = TextureCreateFlagBit::ShaderResource;
    is_reused = true;
}

bool FgRenderPass::add_color_input(const std::string& name) {
    if (!_fg->contains_tex_res(name)) {
        return false;
    }
    auto tex_res = _fg->get_tex_res(name);
    _color_inputs.emplace_back(name);
    _inputs.emplace_back(name);
    tex_res->add_read_pass(get_name());
    tex_res->add_img_usage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    // tex_res->add_img_usage(VK_IMAGE_USAGE_SAMPLED_BIT);
    return true;
}

void FgRenderPass::add_color_output(const std::string &output_name, const AttachmentInfo &attachment_info) {
    std::string name = output_name;
    assert(_fg->contains_tex_res(name) == false);
    auto tex = _fg->get_tex_res(name);
    tex->add_img_usage(TextureCreateFlagBit::RenderTargetable);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    tex->set_attachment_info(attachment_info);
    _color_outputs.emplace_back(name);
    tex->set_write_pass(get_name());
}

/*bool FgRenderPass::add_reslove_input(const std::string &name) {
    if (!_fg->contains_tex_res(name)) {
        return false;
    }
    return true;
}*/

FgRenderPass::~FgRenderPass() {
    // FgRenderpassSetupData _setup_data;
    // FrameGraph* _fg;
    // std::string _name;

    _render_pass = nullptr;
    _group = nullptr;
}

void FgRenderPass::add_reslove_output(const std::string &name, const AttachmentInfo &attachment_info) {
    assert(_fg->contains_tex_res(name) == false);
    auto tex = _fg->get_tex_res(name);
    _resloved_outputs.emplace_back(name);
    tex->add_img_usage(TextureCreateFlagBit::RenderTargetable);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
    tex->set_attachment_info(attachment_info);
    tex->set_write_pass(get_name());
}

bool FgRenderPass::add_depth_stencil_input(const std::string &name) {
    if (!_fg->contains_tex_res(name)) {
        return false;
    }
    _depth_stencil_inputs.emplace_back(name);
    _inputs.emplace_back(name);
    auto tex_res = _fg->get_tex_res(name);
    tex_res->add_read_pass(get_name());
    // tex_res->add_img_usage(VK_IMAGE_USAGE_SAMPLED_BIT);
    tex_res->add_img_usage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    return true;
}

void FgRenderPass::add_depth_stencil_output(const std::string &name, const AttachmentInfo &attachment_info) {
    assert(_fg->contains_tex_res(name) == false);
    auto tex = _fg->get_tex_res(name);
    tex->add_img_usage(TextureCreateFlagBit::DepthStencilTargetable);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    // tex->add_img_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
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
    // if (resloved_outputs.size() > 0) {
    outputs.insert(resloved_outputs.begin(), resloved_outputs.end());
    // }
    // else {
    auto &color_outputs = get_color_outputs();
    outputs.insert(color_outputs.begin(), color_outputs.end());
    // }
    
    auto &depth_stencil_outputs = get_depth_stencil_outputs();
    outputs.insert(depth_stencil_outputs.begin(), depth_stencil_outputs.end());
    return outputs;
}

void FgRenderPass::prepare(std::shared_ptr<Device> device) {
    _setup_data.renderer_interface->prepare_renderpass(*_setup_data.pp_scene, this, device);
}

void FgRenderPass::execute(std::shared_ptr<CommandBuffer> cmd_buf,
                            std::shared_ptr<Device> device) {
    _setup_data.renderer_interface->render_scene(*_setup_data.pp_scene, cmd_buf);
}

void FgRenderPass::set_pipeline_mgr(std::shared_ptr<PipelineManager> mgr) {
    mgr->set_pipeline_create(_setup_data.renderer_interface->get_pipeline_creator());
    _setup_data.renderer_interface->set_pipeline_mgr(mgr);
}


VkSampleCountFlagBits FgRenderPass::get_sample_count() {
    if (_color_outputs.size() > 0) {
        return _fg->get_tex_res(*_color_outputs.begin())->get_attachment_info().samples;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

VkExtent2D FgRenderPass::get_display_size() {
    if (_color_outputs.size() > 0) {
        auto& attach_info =  _fg->get_tex_res(*_color_outputs.begin())->get_attachment_info();
        return VkExtent2D {attach_info.width, attach_info.height};
    }
    auto& attach_info =  _fg->get_tex_res(*_depth_stencil_outputs.begin())->get_attachment_info();
    return VkExtent2D {attach_info.width, attach_info.height};
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

bool FgRenderPassGroup::is_output_res(const std::string& res_name) const{
    
        for (auto output : _output_res) {
            if (output == res_name) {
                return true;
            }
        }
        return false;   
}

void FgRenderPassGroup::insert_pass(const std::string& pass_name) {
    _sub_passes.insert(pass_name);
    auto sub_pass = _fg->_render_pass_map[pass_name];
    auto tex_samples = sub_pass->get_texture_samples();
    _input_res.insert(tex_samples.begin(), tex_samples.end());
#ifdef WIN32
    for (auto& item : sub_pass->get_outputs())
    {
        _output_res.insert(item);
}
#else
    _output_res.insert(sub_pass->get_outputs().begin(), sub_pass->get_outputs().end());
#endif // WIN32


}

bool FgRenderPassGroup::is_depended(std::shared_ptr<FgRenderPassGroup> group) const {
    for (auto input : _input_res) {
        if (group->is_output_res(input)) {
            return true;
        }
    }
    return false;
}

void FgRenderPassGroup::order_pass() {
    auto dependencies = fetch_dependencies();

    std::unordered_map<std::string, uint16_t> depended_count_map;
    for (auto it : dependencies) {
        depended_count_map.insert(std::make_pair(it.first, 0));
    }

    for (auto it : dependencies) {
        for (auto depend_pass_name : it.second) {
            ++depended_count_map[depend_pass_name];
        }
    }

    while(depended_count_map.size()) {
        bool find_zero_depended_pass = false;
        std::string pass_name = "";
        for (auto it : depended_count_map) {
            if (it.second) {
                continue;
            }
            pass_name = it.first;
            find_zero_depended_pass = true;
            break; 
        }
        assert(find_zero_depended_pass);
        if (!find_zero_depended_pass) {
            break;
        }

        _ordered_passes.emplace_back(pass_name);
        depended_count_map.erase(pass_name);
        const std::unordered_set<std::string> &depend_pass_name_set = dependencies[pass_name];
        for (auto &depend_pass_name: depend_pass_name_set) {
            --depended_count_map[depend_pass_name];
        }
    }
    
    std::reverse(_ordered_passes.begin(), _ordered_passes.end());
}

std::unordered_map<std::string, std::unordered_set<std::string>> FgRenderPassGroup::fetch_dependencies() const {
    std::unordered_map<std::string, std::unordered_set<std::string>> dependencies;
    for (auto pass_name : _sub_passes) {
        auto pass = _fg->_render_pass_map[pass_name];
        auto& inputs = pass->get_inputs();
        std::unordered_set<std::string> dependency;
        for (auto input : inputs) {
            auto& input_pass_name = _fg->_tex_res_map[input]->get_write_pass();
            if (_sub_passes.find(input_pass_name) != _sub_passes.end()) {
                dependency.insert(input_pass_name);
            }
        }
        dependencies.insert({pass_name, dependency});
    }
    return dependencies;
}

FgRenderPassGroup::~FgRenderPassGroup() {
    _img_views.clear();
    _imgs.clear();
    _render_pass = nullptr;
    _frame_buffers.clear();
}

void FgRenderPassGroup::bake(std::shared_ptr<Device> device, 
                        std::shared_ptr<Swapchain> swapchain) {

    order_pass();

    // create_image_res(device, swapchain);

    // create_subpass(device, swapchain);

    // create_subpass_dependencies(device, swapchain);

    // create_renderpass(device, swapchain);

    // create_pipeline_mgrs(device, swapchain);

    // create_framebuffer(device, swapchain);

}

void FgRenderPassGroup::create_image_res(std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain) {
    
    _img_views.resize(swapchain->get_display_image_views().size());
    // _attachment_descs.resize(_img_views.size());
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        std::shared_ptr<FgRenderPass> pass = _fg->_render_pass_map[_ordered_passes[i]];
        std::unordered_set<std::string> outputs = pass->get_outputs();
        bool is_swapchain = (_ordered_passes[i] == _fg->_swapchain_pass_name);
        for (auto& output : outputs) {
            create_image(output, device, swapchain, is_swapchain);
            create_attachment(output, device, swapchain, is_swapchain);
        } 
    }
}

void FgRenderPassGroup::create_attachment(const std::string &res_name,
                                   std::shared_ptr<Device> device,
                                   std::shared_ptr<Swapchain> swapchain,
                                   bool is_swapchain_res) {
    std::shared_ptr<FgRenderTextureResource> tex_res = _fg->_tex_res_map[res_name];
    const AttachmentInfo& atta_info = tex_res->get_attachment_info();

    _clear_vals.emplace_back(atta_info.clear_val);

    bool use_as_depth_stencil_atta =
            atta_info.img_usage & TextureCreateFlagBit::DepthStencilTargetable;

    bool use_as_color_atta =
            atta_info.img_usage & TextureCreateFlagBit::RenderTargetable;

    VkAttachmentDescription attachment_desc{};
    attachment_desc.flags = 0;
    attachment_desc.format = atta_info.fmt;
    attachment_desc.samples = atta_info.samples;
    attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    attachment_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    // 如果后续不再使用，final Layout 为 VK_IMAGE_LAYOUT_GENERAL
    // 如果后续直接输出到显示器， final Layout 为 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    // 如果后续需要使用 final layout 为 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    if (use_as_color_atta) {
        // last pass
        if (is_swapchain_res && atta_info.samples == SampleCount::SC_COUNT_1) {
            attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        else if ((!is_swapchain_res) && atta_info.samples == SampleCount::SC_COUNT_1){
            attachment_desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }
    
    if (use_as_depth_stencil_atta &&
        ((atta_info.img_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) !=
                                                                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
        attachment_desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL/*VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL*/;
    }

    attachment_desc.stencilLoadOp =  VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc.stencilStoreOp =  VK_ATTACHMENT_STORE_OP_DONT_CARE;

    attachment_desc.loadOp = atta_info.load_op;
    attachment_desc.storeOp = atta_info.store_op;
   
    _attachment_descs.emplace_back(attachment_desc);
}

void FgRenderPassGroup::create_image(const std::string& res_name, 
            std::shared_ptr<Device> device, std::shared_ptr<Swapchain> swapchain, 
            bool is_swapchain_res) {
    std::shared_ptr<FgRenderTextureResource> tex_res = _fg->_tex_res_map[res_name];
    const AttachmentInfo& atta_info = tex_res->get_attachment_info();
    bool use_as_depth_stencil_atta =
            atta_info.img_usage & TextureCreateFlagBit::DepthStencilTargetable;

    bool use_as_color_atta =
            atta_info.img_usage & TextureCreateFlagBit::RenderTargetable;

    auto& display_views = swapchain->get_display_image_views();

    if (is_swapchain_res &&
            use_as_color_atta &&
            atta_info.samples == SampleCount::SC_COUNT_1) {
        uint16_t idx = _img_views[0].size();
        _tex_index_map.insert({res_name, idx});
        for (uint8_t i = 0; i < display_views.size(); i++) {
            _img_views[i].emplace_back(display_views[i]);
        }
        
        return;
    }

    VkImageAspectFlags bits = VK_IMAGE_ASPECT_COLOR_BIT;

    if (use_as_depth_stencil_atta) {
        bits = (VK_IMAGE_ASPECT_DEPTH_BIT /*| VK_IMAGE_ASPECT_STENCIL_BIT*/);
    }                                
    
    uint16_t idx = _img_views[0].size();
    _tex_index_map.insert({res_name, idx});

    for (uint8_t i = 0; i < display_views.size(); i++) {
        uint8_t img_idx = atta_info.is_reused ? i : 0;
        std::shared_ptr<Image> img = _fg->get_image(atta_info.img_name, img_idx);
        std::shared_ptr<ImageView> img_view = std::make_shared<ImageView>(device, img, atta_info.fmt, 
                                                    VK_IMAGE_VIEW_TYPE_2D, bits,
                                                    atta_info.level, atta_info.layer);
        _fg->add_image_view(res_name, img_view);                                            
        _img_views[i].emplace_back(img_view);
    }
}

/*
void FgRenderPassGroup::create_subpass(std::shared_ptr<Device> device, 
                            std::shared_ptr<Swapchain> swapchain) {
    _attachment_refs = std::vector<std::vector<VkAttachmentReference>>(1000,
                                                       std::vector<VkAttachmentReference>(0));
    std::unordered_map<std::string, uint16_t> tex_use_count_map;
    
    std::vector<uint32_t> preserve_attachments;

    auto remove_preserve_attachment = [&preserve_attachments](uint16_t atta) {
        auto it = std::find(preserve_attachments.begin(), preserve_attachments.end(), atta);
        if (it != preserve_attachments.end()) {
            preserve_attachments.erase(it);
        }
    };

    auto add_preserve_attachment = [&preserve_attachments](uint16_t atta) {
        preserve_attachments.emplace_back(atta);
    };

    uint16_t used_num = 0;
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        auto& pass = _fg->_render_pass_map[_ordered_passes[i]];
        
        bool is_swapchain_pass = (_ordered_passes[i] == _fg->_swapchain_pass_name);

        const std::vector<std::string>& inputs = pass->get_inputs();
        // _attachment_refs.emplace_back(std::vector<VkAttachmentReference>(0));
        std::vector<VkAttachmentReference>& input_refs = _attachment_refs[used_num++];
        for (uint16_t i = 0; i < inputs.size(); i++) {
            auto& tex_name = inputs[i];
            auto attach_info = _fg->_tex_res_map[tex_name]->get_attachment_info();

            // 判断是否为input attachment
            if (!(attach_info.img_usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
                continue;
            }

            if (tex_use_count_map.find(tex_name) == tex_use_count_map.end()) {
                tex_use_count_map.insert({tex_name, 0});
            }
            ++tex_use_count_map[tex_name];
            uint16_t tex_idx = _tex_index_map[tex_name];

            if (tex_use_count_map[tex_name] == _fg->_tex_res_map[tex_name]->get_read_passes().size()) {
                remove_preserve_attachment(tex_idx);
            }
            VkAttachmentReference atta_ref{};
            atta_ref.attachment = tex_idx;
            atta_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            input_refs.emplace_back(atta_ref);
        }
        
        _preserved_attachments_arr.emplace_back(preserve_attachments);

        const std::vector<std::string>& resloved_outputs = pass->get_resloved_outputs();
        // _attachment_refs.emplace_back(std::vector<VkAttachmentReference>(0));
        std::vector<VkAttachmentReference>& resloved_refs = _attachment_refs[used_num++];
        uint16_t resloved_idx = _attachment_refs.size() - 1;
        for (uint16_t i = 0; i < resloved_outputs.size(); i++) {
            auto& tex_name = resloved_outputs[i];
            uint16_t tex_idx = _tex_index_map[tex_name];
            if (_fg->_tex_res_map[tex_name]->get_read_passes().size() > 0) {
                add_preserve_attachment(tex_idx);
            }
            VkAttachmentReference atta_ref{};
            atta_ref.attachment = tex_idx;
            atta_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            if (is_swapchain_pass) {
                atta_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            resloved_refs.emplace_back(atta_ref);
        }
        
        // std::vector<VkAttachmentReference> color_refs;
        const std::vector<std::string>& color_outputs = pass->get_color_outputs();
        // _attachment_refs.emplace_back(std::vector<VkAttachmentReference>(0));
        std::vector<VkAttachmentReference>& color_refs = _attachment_refs[used_num++];
        for (uint16_t i = 0; i < color_outputs.size(); i++) {
            auto& tex_name = color_outputs[i];
            uint16_t tex_idx = _tex_index_map[tex_name];

            if (resloved_refs.size() == 0 &&
            _fg->_tex_res_map[tex_name]->get_read_passes().size() > 0) {
                add_preserve_attachment(tex_idx);
            }
            VkAttachmentReference atta_ref{};
            atta_ref.attachment = tex_idx;
            atta_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            if (is_swapchain_pass && resloved_outputs.size() == 0) {
                atta_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            
            color_refs.emplace_back(atta_ref);
        }
        
        
        const std::vector<std::string>& depth_stencil_outputs = pass->get_depth_stencil_outputs();
        // _attachment_refs.emplace_back(std::vector<VkAttachmentReference>(0));
        std::vector<VkAttachmentReference>& depth_stencil_refs = _attachment_refs[used_num++];
        for (uint16_t i = 0; i < depth_stencil_outputs.size(); i++) {
            auto& tex_name = depth_stencil_outputs[i];
            uint16_t tex_idx = _tex_index_map[tex_name];

            if (_fg->_tex_res_map[tex_name]->get_read_passes().size() > 0) {
                add_preserve_attachment(tex_idx);
            }
            VkAttachmentReference atta_ref{};
            atta_ref.attachment = tex_idx;
            atta_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_stencil_refs.emplace_back(atta_ref);
        }
        

        auto& tmp_preserved_attachments = _preserved_attachments_arr[_preserved_attachments_arr.size() - 1];

        VkSubpassDescription subpass_desc{};
        subpass_desc.flags = 0;
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // if (inputs.size() > 0) {
        if (false) {
            subpass_desc.inputAttachmentCount = input_refs.size();
            subpass_desc.pInputAttachments = input_refs.data();
        }
        else {
            subpass_desc.inputAttachmentCount = 0;
            subpass_desc.pInputAttachments = nullptr;
        }
        if (resloved_outputs.size() > 0) {
            subpass_desc.pResolveAttachments = resloved_refs.data();
        }
        else {
            subpass_desc.pPreserveAttachments = nullptr;
        }

        if (color_outputs.size() > 0) {
            subpass_desc.colorAttachmentCount = color_refs.size();
            subpass_desc.pColorAttachments = color_refs.data();
        }
        else {
            subpass_desc.colorAttachmentCount = 0;
            subpass_desc.pColorAttachments = nullptr;
        }

        if (depth_stencil_outputs.size() > 0) {
            subpass_desc.pDepthStencilAttachment = depth_stencil_refs.data();
        }
        else {
            subpass_desc.pDepthStencilAttachment = nullptr;
        }

        if (preserve_attachments.size() > 0) {
            subpass_desc.preserveAttachmentCount = tmp_preserved_attachments.size();
            subpass_desc.pPreserveAttachments = tmp_preserved_attachments.data();
        }
        else {
            subpass_desc.preserveAttachmentCount = 0;
            subpass_desc.pPreserveAttachments = nullptr;
        }

        _subpass_descs.emplace_back(subpass_desc);
    }
}*/

/*
void FgRenderPassGroup::create_subpass_dependencies(std::shared_ptr<Device> device,
                                         std::shared_ptr<Swapchain> swapchain) {
    auto find_src_pass_idx = [this](const std::string& src_pass,
            uint16_t dst_pass_idx) -> int {
        int i = dst_pass_idx - 1;
        for (; i >= 0; i--) {
            if (this->_ordered_passes[i] == src_pass) {
                break;
            }
        }

        return i;
    } ;

    auto dependencies = fetch_dependencies();
    for (int i = _ordered_passes.size() - 1; i >= 0; i--) {
        auto dst_pass = _ordered_passes[i];
        uint16_t dst_pass_idx = i;
        if (dependencies.find(dst_pass) == dependencies.end()) {
            continue;
        }
        auto src_passes = dependencies[dst_pass];
        if (src_passes.size() == 0) {
            VkSubpassDependency dependency{};

            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;

            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            _subpass_dependecies.emplace_back(dependency);
            continue;
        }
        for (auto &src_pass : src_passes) {
            uint16_t src_pass_idx = find_src_pass_idx(src_pass, dst_pass_idx);
            VkSubpassDependency sd{};
            if (src_pass_idx >= 0) {
                sd.srcSubpass = src_pass_idx;
            }
            else {
                sd.srcSubpass = VK_SUBPASS_EXTERNAL;
            }
            sd.dstSubpass = dst_pass_idx;

            if (src_pass_idx >= 0) {
                sd.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                // sd.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }
            else {
                sd.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }

            sd.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            // sd.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

            if (src_pass_idx >= 0) {
                sd.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            else {
                sd.srcAccessMask = 0;
            }
            sd.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            _subpass_dependecies.emplace_back(sd);
        }
    }
}*/

/*
void FgRenderPassGroup::create_renderpass(std::shared_ptr<Device> device,
                                   std::shared_ptr<Swapchain> swapchain) {
    VkRenderPassCreateInfo rpc {};
    rpc.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpc.pNext = nullptr;
    rpc.flags = 0;
    rpc.pSubpasses = _subpass_descs.data();
    rpc.subpassCount = _subpass_descs.size();
    rpc.pAttachments = _attachment_descs.data();
    rpc.attachmentCount = _attachment_descs.size();
    rpc.pDependencies = _subpass_dependecies.data();
    rpc.dependencyCount = _subpass_dependecies.size();

    auto fg_render_pass = _fg->_render_pass_map[*_sub_passes.begin()];
    auto extent = fg_render_pass->get_display_size();

    _render_pass = std::make_shared<RenderPass>(device, extent, rpc);
    // _render_pass = std::make_shared<RenderPass>(device, swapchain, true, true, VK_SAMPLE_COUNT_4_BIT);
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        _fg->_render_pass_map[_ordered_passes[i]]->set_render_pass(_render_pass);
    }
    // _render_pass = std::make_shared<RenderPass>(device, swapchain, true, true);
    // _pipeline_manager = std::make_shared<PipelineManager>(device, _render_pass, 
    //                                 swapchain->get_display_size() );
}
*/
void FgRenderPassGroup::create_pipeline_mgrs(std::shared_ptr<Device> device, 
                                    std::shared_ptr<Swapchain> swapchain) {
    for (uint32_t i = 0; i < _ordered_passes.size(); i++) {
        auto& pass_name = _ordered_passes[i];
        auto pass = _fg->_render_pass_map[pass_name];
        
        std::shared_ptr<PipelineManager> pipeline_mgr = std::make_shared<PipelineManager>(
            device, pass, i);
        pass->set_pipeline_mgr(pipeline_mgr);    
    }
}

/*void FgRenderPassGroup::create_framebuffer(std::shared_ptr<Device> device, 
                            std::shared_ptr<Swapchain> swapchain) {

    auto& display_views = swapchain->get_display_image_views();
    auto fg_render_pass = _fg->_render_pass_map[*_sub_passes.begin()];
    auto extent = fg_render_pass->get_display_size();
    for (uint8_t i = 0; i < display_views.size(); i++) {
        std::shared_ptr<Framebuffer> fb = std::make_shared<Framebuffer>(device,
                           _render_pass.get(), fg_render_pass->get_display_size(),
                                                 _img_views[i]);
        _frame_buffers.emplace_back(fb);
    }
    // _frame_buffers = _render_pass->get_framebuffers();
}
*/
void FgRenderPassGroup::prepare_renderpasses(std::shared_ptr<Device> device) {
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        auto& pass_name = _ordered_passes[i];
        auto pass = _fg->_render_pass_map[pass_name];
        
        pass->prepare(device);
    }
}

void FgRenderPassGroup::execute(std::shared_ptr<Device> device, uint16_t active_frame_idx, 
                std::shared_ptr<CommandBuffer> cmd_buf) {
    // auto cmd_buf = _fg->_cmd_bufs[active_frame_idx];
    auto frame_buf = _frame_buffers[active_frame_idx];
    // cmd_buf->begin();
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        auto& pass_name = _ordered_passes[i];
        auto pass = _fg->_render_pass_map[pass_name];
        if (i == 0) {
            _render_pass->begin(cmd_buf, frame_buf, _clear_vals);

        }
        else {
            _render_pass->next(cmd_buf);
        }
        pass->execute(cmd_buf, device);
    }
    _render_pass->end(cmd_buf);
    // cmd_buf->end();
}

/*std::shared_ptr<ImageView> FgRenderPassGroup::get_image_view(const std::string& tex_name) {
    if (_tex_index_map.find(tex_name) == _tex_index_map.end()) {
        return nullptr;
    }
    uint16_t tex_idx = _tex_index_map[tex_name];
    return _img_views[_fg->_active_frame_idx][tex_idx];
}*/




