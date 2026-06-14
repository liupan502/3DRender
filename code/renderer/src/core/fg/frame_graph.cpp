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

#include <rhi/rhi_resource.h>
#include <texture_cache.h>


using namespace zr::core;

bool FrameGraph::contains_tex_res(const std::string &name) const {
    return !(_tex_res_map.find(name) == _tex_res_map.end());
}

std::shared_ptr<FgRenderTextureResource> FrameGraph::get_tex_res(const std::string& name) {
    if (_tex_res_map.find(name) == _tex_res_map.end()) {
        _tex_res_map.insert(std::make_pair(name, std::make_shared<FgRenderTextureResource>()));
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

void FrameGraph::bake() {
    if (!need_bake()) {
        return;
    }

    create_images(/*swapchain->get_display_image_views().size()*/ 1);
    create_images(/*swapchain->get_display_image_views().size()*/ 1);

    order_passes();

    compute_texture_lifetimes();

    _need_bake = false;
}

void FrameGraph::execute() {
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        auto& pass = _render_pass_map[_ordered_passes[i]];

        PassResources res;

        const std::vector<std::string>& inputs = pass->get_inputs();
        for (auto& tex_name : inputs) {
            if (_tex_res_map.find(tex_name) == _tex_res_map.end()) continue;
            auto& tex_res = _tex_res_map[tex_name];
            auto& img_name = tex_res->get_attachment_info().img_name;
            auto it = _tex_handle_map.find(img_name);
            if (it != _tex_handle_map.end()) {
                res.input_textures.emplace_back(it->second);
            }
        }

        rhi::RenderTargetCreateInfo ci;
        auto outputs = pass->get_outputs();
        for (auto& tex_name : outputs) {
            if (_tex_res_map.find(tex_name) == _tex_res_map.end()) continue;
            auto& tex_res = _tex_res_map[tex_name];
            auto& attach_info = tex_res->get_attachment_info();
            auto& img_name = attach_info.img_name;

            auto it = _tex_handle_map.find(img_name);
            if (it == _tex_handle_map.end()) {
                rhi::TextureCreateInfo ci;
                ci.width = attach_info.width;
                ci.height = attach_info.height;
                ci.depth = attach_info.depth;
                ci.layer_num = attach_info.layer_num;
                ci.mip_num = attach_info.mip_num;
                ci.format = attach_info.fmt;
                ci.flags = attach_info.img_usage;

                auto img = TexturePool::instance().acquire(ci);
                _tex_handle_map[img_name] = img;
            }

            rhi::Attachment attachment = { attach_info, _tex_handle_map[img_name] };
            if (attach_info.img_usage & rhi::TextureCreateFlagBit::RenderTargetable) {
                ci.color_attachments.emplace_back(std::move(attachment));
            }
            else if (attach_info.img_usage & rhi::TextureCreateFlagBit::DepthStencilTargetable){
                ci.depth_attachment = std::move(attachment);
            }

            res.render_target = rhi::rhi_instance->create_render_target(ci);
        }

        pass->prepare(res);
        pass->execute(res);

        for (auto& pair : _tex_res_map) {
            auto& tex_res = pair.second;
            if (tex_res->get_last_pass_idx() == i) {
                auto& img_name = tex_res->get_attachment_info().img_name;
                auto it = _tex_handle_map.find(img_name);
                if (it != _tex_handle_map.end()) {
                    TexturePool::instance().release(it->second);
                    _tex_handle_map.erase(it);
                }
            }
        }
    }
}

void FrameGraph::order_passes() {
    std::unordered_map<std::string, std::unordered_set<std::string>> dependencies;

    for (auto& pair : _render_pass_map) {
        auto& pass_name = pair.first;
        auto& pass = pair.second;
        std::unordered_set<std::string> dependency;

        const std::vector<std::string>& inputs = pass->get_inputs();
        for (auto& input : inputs) {
            if (_tex_res_map.find(input) == _tex_res_map.end()) {
                continue;
            }
            auto& write_pass = _tex_res_map[input]->get_write_pass();
            if (!write_pass.empty() && write_pass != pass_name) {
                dependency.insert(write_pass);
            }
        }

        const std::vector<std::string>& tex_samples = pass->get_texture_samples();
        for (auto& tex_name : tex_samples) {
            if (_tex_res_map.find(tex_name) == _tex_res_map.end()) {
                continue;
            }
            auto& write_pass = _tex_res_map[tex_name]->get_write_pass();
            if (!write_pass.empty() && write_pass != pass_name) {
                dependency.insert(write_pass);
            }
        }

        dependencies.insert({pass_name, dependency});
    }

    std::unordered_map<std::string, uint16_t> depended_count_map;
    for (auto& it : dependencies) {
        depended_count_map.insert(std::make_pair(it.first, 0));
    }

    for (auto& it : dependencies) {
        for (auto& depend_pass_name : it.second) {
            if (depended_count_map.find(depend_pass_name) != depended_count_map.end()) {
                ++depended_count_map[depend_pass_name];
            }
        }
    }

    while (depended_count_map.size()) {
        bool found = false;
        std::string pass_name;
        for (auto& it : depended_count_map) {
            if (it.second == 0) {
                pass_name = it.first;
                found = true;
                break;
            }
        }
        assert(found);

        _ordered_passes.emplace_back(pass_name);
        depended_count_map.erase(pass_name);
        const std::unordered_set<std::string>& deps = dependencies[pass_name];
        for (auto& depend_pass_name : deps) {
            if (depended_count_map.find(depend_pass_name) != depended_count_map.end()) {
                --depended_count_map[depend_pass_name];
            }
        }
    }

    std::reverse(_ordered_passes.begin(), _ordered_passes.end());
}

void FrameGraph::compute_texture_lifetimes() {
    std::unordered_map<std::string, uint16_t> pass_index_map;
    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        pass_index_map[_ordered_passes[i]] = i;
    }

    for (auto& pair : _tex_res_map) {
        auto& tex_res = pair.second;
        tex_res->set_first_pass_idx(UINT16_MAX);
        tex_res->set_last_pass_idx(0);
    }

    for (uint16_t i = 0; i < _ordered_passes.size(); i++) {
        auto& pass = _render_pass_map[_ordered_passes[i]];

        auto outputs = pass->get_outputs();
        for (auto& tex_name : outputs) {
            if (_tex_res_map.find(tex_name) == _tex_res_map.end()) continue;
            auto& tex_res = _tex_res_map[tex_name];
            if (i < tex_res->get_first_pass_idx()) {
                tex_res->set_first_pass_idx(i);
            }
        }

        const std::vector<std::string>& tex_samples = pass->get_texture_samples();
        for (auto& tex_name : tex_samples) {
            if (_tex_res_map.find(tex_name) == _tex_res_map.end()) continue;
            auto& tex_res = _tex_res_map[tex_name];
            if (i > tex_res->get_last_pass_idx()) {
                tex_res->set_last_pass_idx(i);
            }
        }

        const std::vector<std::string>& inputs = pass->get_inputs();
        for (auto& tex_name : inputs) {
            if (_tex_res_map.find(tex_name) == _tex_res_map.end()) continue;
            auto& tex_res = _tex_res_map[tex_name];
            if (i > tex_res->get_last_pass_idx()) {
                tex_res->set_last_pass_idx(i);
            }
        }
    }

    for (auto& pair : _tex_res_map) {
        auto& tex_res = pair.second;
        if (tex_res->get_first_pass_idx() == UINT16_MAX) {
            const auto& write_pass = tex_res->get_write_pass();
            if (!write_pass.empty() && pass_index_map.find(write_pass) != pass_index_map.end()) {
                tex_res->set_first_pass_idx(pass_index_map[write_pass]);
            }
        }
        if (tex_res->get_last_pass_idx() < tex_res->get_first_pass_idx()) {
            tex_res->set_last_pass_idx(tex_res->get_first_pass_idx());
        }
    }
}

void FrameGraph::create_images(uint8_t swapchain_num) {
    std::unordered_map<std::string, rhi::AttachmentInfo> img_infos;
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
        std::vector<std::shared_ptr<rhi::Texture>> imgs;
        uint8_t count = atta_info.is_reused ? swapchain_num : 1;
        for (uint8_t i = 0; i < count; i++) {
            rhi::TextureCreateInfo ci;
            ci.width = atta_info.width;
            ci.height = atta_info.height;
            ci.depth = atta_info.depth;
            ci.layer_num = atta_info.layer + 1;
            ci.mip_num = atta_info.level + 1;
            ci.format = atta_info.fmt;
            ci.flags = atta_info.img_usage;

            auto img = TexturePool::instance().acquire(ci);
            imgs.emplace_back(img);
        }
        
        _imgs_map.insert({img_info.first, imgs});                                
    }
}

void FrameGraph::add_image_view(const std::string& tex_name, std::shared_ptr<ImageView> img_view) {
    _image_view_map[tex_name] = img_view;
}

std::shared_ptr<ImageView> FrameGraph::get_image_view(const std::string& tex_name) {
    auto it = _image_view_map.find(tex_name);
    if (it == _image_view_map.end()) return nullptr;
    return it->second;
}

rhi::TextureRef FrameGraph::get_texture_handle(const std::string& tex_name) {
    auto it = _tex_res_map.find(tex_name);
    if (it == _tex_res_map.end()) return nullptr;
    auto& img_name = it->second->get_attachment_info().img_name;
    auto hit = _tex_handle_map.find(img_name);
    if (hit == _tex_handle_map.end()) return nullptr;
    return hit->second;
}

std::shared_ptr<rhi::Texture> FrameGraph::get_image(const std::string& img_name, uint8_t idx) {
    if (_imgs_map.find(img_name) == _imgs_map.end()) {
        return nullptr;
    }

    assert(_imgs_map[img_name].size() > idx);
    return _imgs_map[img_name][idx];
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
    if (_ordered_passes.size() > 0) {
        _ordered_passes.clear();
    }
    if (_tex_handle_map.size() > 0) {
        _tex_handle_map.clear();
    }
    if (_image_view_map.size() > 0) {
        _image_view_map.clear();
    }
}

FrameGraph::~FrameGraph() {
   reset();
}
