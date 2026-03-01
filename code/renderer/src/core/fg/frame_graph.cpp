#include <core/fg/frame_graph.h>
#include <core/device.h>
#include <core/swapchain.h>
#include <core/image.h>
#include <core/image_view.h>
#include <core/framebuffer.h>
#include <core/render_pass.h>
#include <core/command_buffer.h>
#include <core/command_pool.h>
#include <core/pipeline_manager.h>
#include <core/renderers/renderer_interface.h>
#include <string.h>
#include <algorithm>


using namespace zr::core;

bool FrameGraph::contains_tex_res(const std::string &name) const {
    return !(_tex_res_map.find(name) == _tex_res_map.end());
}

std::shared_ptr<FgRenderTextureResource> FrameGraph::get_tex_res(const std::string& name) {
    if (_tex_res_map.find(name) == _tex_res_map.end()) {
        _tex_res_map.insert(std::make_pair(name, std::make_shared<FgRenderTextureResource>()));
        // _tex_res_map[name]->add_img_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        // _tex_res_map[name]->add_img_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }
    return _tex_res_map[name];
}

std::shared_ptr<FgRenderPass> FrameGraph::add_pass(const std::string &pass_name, bool is_swapchain) {
    if (_render_pass_map.find(pass_name) == _render_pass_map.end()) {
        _render_pass_map.insert(std::make_pair(pass_name, std::make_shared<FgRenderPass>(this, pass_name)));
    }
    if (is_swapchain) {
        _swapchain_pass_name = pass_name;
    }
    return _render_pass_map[pass_name];
}

void FrameGraph::seperate_render_pass() {
    std::unordered_set<std::string> start_pass_names;
    start_pass_names.insert(_swapchain_pass_name);
    std::unordered_set<std::string> used_pass_names;
    while(true) {
        if (start_pass_names.size() == 0) {
            break;
        }
        auto pass_name = *start_pass_names.begin();
        auto pass = _render_pass_map[pass_name];
        // start_pass_names.erase(start_pass_names.begin());

        std::shared_ptr<FgRenderPassGroup> group = std::make_shared<FgRenderPassGroup>(this);
        _groups.emplace_back(group);
        // group->insert_pass(pass_name);

        std::unordered_set<std::string> tmp_pass_names;
        tmp_pass_names.insert(pass_name);
        auto fetch_pass = [this](std::unordered_set<std::string>& pass_set, const std::vector<std::string>& res_names) {
            for (uint16_t i = 0; i < res_names.size(); i++) {
                if (!contains_tex_res(res_names[i])) {
                    continue;
                }
                auto tex = get_tex_res(res_names[i]);
                std::unordered_set<std::string> tmp_passes = tex->get_read_passes();
                std::unordered_set<std::string> passes;
                for (auto& pass_name : tmp_passes) {
                    auto tex_samples = _render_pass_map[pass_name]->get_texture_samples();
                    bool is_tex_sample = false;
                    for (auto& tex_sample_name : tex_samples) {
                        if (tex_sample_name == res_names[i]) {
                            is_tex_sample = true;
                            break;
                        }
                    }
                    if (is_tex_sample) {
                        continue;
                    }
                    passes.insert(pass_name);
                }
                passes.insert(tex->get_write_pass());
               
                pass_set.insert(passes.begin(), passes.end());
            }
        };

        while(true) {
            if (tmp_pass_names.size() == 0 ) {
                break;
            }
            auto tmp_pass_name = *tmp_pass_names.begin();
            
            if (used_pass_names.find(tmp_pass_name) != used_pass_names.end()) {
                tmp_pass_names.erase(tmp_pass_name);
                continue;
            }

            auto tmp_pass = _render_pass_map[tmp_pass_name];

            // auto color_inputs = tmp_pass->get_color_inputs();
            fetch_pass(tmp_pass_names, tmp_pass->get_color_inputs());

            fetch_pass(tmp_pass_names, tmp_pass->get_depth_stencil_inputs());

            fetch_pass(tmp_pass_names, tmp_pass->get_color_outputs());

            fetch_pass(tmp_pass_names, tmp_pass->get_resloved_outputs());

            fetch_pass(tmp_pass_names, tmp_pass->get_depth_stencil_outputs());

            auto tex_samples = tmp_pass->get_texture_samples();
            for (uint16_t i = 0; i < tex_samples.size(); i++) {
                auto tex = get_tex_res(tex_samples[i]);
                start_pass_names.insert(tex->get_write_pass());
            }

            group->insert_pass(tmp_pass_name);
            used_pass_names.insert(tmp_pass_name);
            _render_pass_map[tmp_pass_name]->set_group(group);
            tmp_pass_names.erase(tmp_pass_name);
        }

        for (auto it = start_pass_names.begin(); it != start_pass_names.end();) {
            if (used_pass_names.find(*it) != used_pass_names.end()) {
                it = start_pass_names.erase(it);
            }
            else {
                it++;
            }
        }
    }
}

void FrameGraph::bake(std::shared_ptr<Device> device, 
                        std::shared_ptr<Swapchain> swapchain,
                        std::shared_ptr<CommandPool> cmd_pool) {
    if (!need_bake()) {
        return;
    }

    create_images(device, swapchain->get_display_image_views().size());

    seperate_render_pass();
    
    order_group();

    for (uint16_t i = 0; i < _groups.size(); i++) {
        _groups[i]->bake(device, swapchain);
    }

    create_cmd_bufs(device, swapchain, cmd_pool);

    _need_bake = false;
}

void FrameGraph::create_cmd_bufs(std::shared_ptr<Device> device, 
                                    std::shared_ptr<Swapchain> swapchain,
                                    std::shared_ptr<CommandPool> cmd_pool) {
    uint32_t count = swapchain->get_display_image_views().size();
    _cmd_bufs.clear();
    for (uint32_t i = 0; i < count; i++) {
        _cmd_bufs.emplace_back(std::make_shared<CommandBuffer>(device, cmd_pool));
    }
}

void FrameGraph::execute(uint16_t active_frame_idx, std::shared_ptr<Device> device) {
    _active_frame_idx = active_frame_idx;

    for (uint16_t i = 0; i < _groups.size(); i++) {
        _groups[i]->prepare_renderpasses(device);
    }

    auto cmd_buf = _cmd_bufs[_active_frame_idx];
    cmd_buf->begin();
    for (uint16_t i = 0; i < _groups.size(); i++) {
        _groups[i]->execute(device, active_frame_idx, cmd_buf);
    }
    cmd_buf->end();
}

std::shared_ptr<ImageView> FrameGraph::get_image_view(const std::string& tex_name) {
    uint32_t idx = 0;
    if (_img_views_map[tex_name].size() > 1) {
        idx = _active_frame_idx;
    }
    return _img_views_map[tex_name][idx];
}

void FrameGraph::add_image_view(const std::string& tex_name, std::shared_ptr<ImageView> img_view) {
    if (_img_views_map.find(tex_name) == _img_views_map.end()) {
        _img_views_map.insert({tex_name, {img_view}});
    }
    else {
        _img_views_map[tex_name].emplace_back(img_view);
    }
}


void FrameGraph::order_group() {
    std::map<int, std::unordered_set<int>> dependencies;

    for (uint16_t i = 0; i < _groups.size(); i++) {
        std::unordered_set<int> dependency;
        for (uint16_t j = 0; j < _groups.size(); j++) {
            if (i == j) {
                continue;
            }
            if (_groups[j]->is_depended(_groups[i])) {
                dependency.insert(j);
            }
        }
        dependencies.insert({i, dependency});
    }

    std::vector<std::shared_ptr<FgRenderPassGroup>> tmp_groups;
    while(true) {
        if (dependencies.size() == 0) {
            break;
        }
        for (auto it = dependencies.begin(); it != dependencies.end(); it++) {
            if (it->second.size() > 0) {
                continue;
            }
            int idx = it->first;
            dependencies.erase(it);
            tmp_groups.emplace_back(_groups[idx]);
            for (auto it = dependencies.begin(); it != dependencies.end(); it++) {
                it->second.erase(idx);
            }
            break;
        }
    }

    _groups = tmp_groups;

    std::reverse(_groups.begin(), _groups.end());
}

void FrameGraph::create_images(std::shared_ptr<Device> device, uint8_t swapchain_num) {
    std::unordered_map<std::string, AttachmentInfo> img_infos;
    for (auto pass : _render_pass_map) {
        if (_swapchain_pass_name == pass.first) {
            continue;
        }

        auto outputs = pass.second->get_outputs();

        for (auto output : outputs) {
            auto tex = _tex_res_map[output];
            auto atta_info = tex->get_attachment_info();
            auto& img_name = atta_info.img_name;
            if (img_infos.find(img_name) == img_infos.end()) {
                img_infos.insert({img_name, atta_info});
            }
            else {

                auto& img_info = img_infos[img_name];

                assert(img_info.fmt == atta_info.fmt);
                assert(img_info.samples == atta_info.samples);
                // assert(img_info.clear_val == attachment_info)
                
                img_info.img_usage = img_info.img_usage | atta_info.img_usage;
                
                if (img_info.layer < atta_info.layer) {
                    img_info.layer = atta_info.layer;
                }
                
                if (img_info.level < atta_info.level) {
                    img_info.level = atta_info.level;
                }
                
                if (img_info.width < atta_info.width) {
                    img_info.width = atta_info.width;
                }

                if (img_info.height < atta_info.height) {
                    img_info.height = atta_info.height;
                }

                if (img_info.depth < atta_info.depth) {
                    img_info.depth = atta_info.depth;
                }
            }
        }
    }

    for (auto img_info : img_infos) {
        auto atta_info = img_info.second;
        VkExtent3D extent;
        extent.width = atta_info.width;
        extent.height = atta_info.height;
        extent.depth = atta_info.depth;
        std::vector<std::shared_ptr<core::Image>> imgs;
        uint8_t count = atta_info.is_reused ? swapchain_num : 1;
        for (uint8_t i = 0; i < count; i++) {
            std::shared_ptr<Image> img = std::make_shared<Image>(device, extent, atta_info.fmt,
                                        atta_info.img_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        atta_info.samples,
                                        atta_info.level + 1, atta_info.layer + 1);
            imgs.emplace_back(img);
        }
        
        _imgs_map.insert({img_info.first, imgs});                                
    }
}

std::shared_ptr<Image> FrameGraph::get_image(const std::string& img_name, uint8_t idx) {
    if (_imgs_map.find(img_name) == _imgs_map.end()) {
        return nullptr;
    }

    assert(_imgs_map[img_name].size() > idx);
    return _imgs_map[img_name][idx];
}

void FrameGraph::set_viewport(VkViewport viewport) {
    for (auto pair : _render_pass_map) {
        pair.second->get_setup_data().renderer_interface->set_viewport(viewport);
    }
}

void FrameGraph::reset() {
    if (_tex_res_map.size() > 0) {
        _tex_res_map.clear();
    }
    if (_imgs_map.size() > 0) {
        _imgs_map.clear();
    }
    if (_render_pass_map.size() > 0) {
        _render_pass_map.clear();
    }
    if (_cmd_bufs.size() > 0) {
        _cmd_bufs.clear();
    }
    if (_groups.size() > 0) {
        _groups.clear();
    }
    if (_img_views_map.size() > 0) {
        _img_views_map.clear();
    }
}

FrameGraph::~FrameGraph() {
   reset();
}
