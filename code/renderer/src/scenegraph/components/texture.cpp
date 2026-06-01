//
// Created by zhida.ji1 on 2022/8/8.
//

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <scenegraph/components/texture.h>
#include <utils/file_helper.h>
#include <core/core.h>
#include <rhi/rhi.h>



using namespace zr::sg;
using namespace zr;

void Texture::update_2d_data(const TextureContent& tc,
                             rhi::TextureRef rhi_texture, uint8_t img_data_idx, bool auto_blit) {
    auto img_data_info = tc.img_data_infos[img_data_idx];
    if (auto_blit && img_data_idx == 0) {

        rhi::rhi_instance->update_texture(rhi_texture, img_data_info.ptr,
                              img_data_info.data_len, 
                              img_data_info.layer_idx, 0, auto_blit);
    }
    else {
        rhi::rhi_instance->update_texture(rhi_texture, img_data_info.ptr,
                              img_data_info.data_len, 
                              img_data_info.layer_idx, img_data_info.mipmap_level, auto_blit);
    }
    // tc.image->update_data(img_data_info.ptr, img_data_info.width * img_data_info.height * 4, 0);
}

void Texture::get_cube_face_offset(uint16_t size, uint8_t face, uint16_t& row_num, uint16_t& col_num) const {
    row_num = 0;
    col_num = 0;
    switch (face) {
    case 0: // x+
        row_num = size;
        col_num = size * 2;
        break;
    case 1: // x-
        row_num = size;
        col_num = 0;
        break;
    case 2: // y+
        row_num = 0;
        col_num = size;
        break;
    case 3: // y-
        row_num = size * 2;
        col_num = size;
        break;
    case 5: // z+
        row_num = size;
        col_num = size * 3;
        break;
    case 4: // z-
        row_num = size;
        col_num = size;
        break;
    default:
        break;
    }
}

void Texture::update_cube_data(const TextureContent& tc, 
                                rhi::TextureRef rhi_texture, uint8_t img_data_idx,
                                bool auto_blit) {
    auto img_data_info = tc.img_data_infos[img_data_idx];
    assert(img_data_info.width / 4 == img_data_info.height / 3);
    uint16_t size = img_data_info.width / 4;
    unsigned char* buf = new unsigned char[size * size * img_data_info.com];

    for (uint8_t i = 0; i < 6; i++) {
        uint16_t row = 0;
        uint16_t col = 0;
        get_cube_face_offset(size, i, row, col);
        uint32_t dst_offset = 0;
        for (uint16_t j = 0; j < size; j++) {
            uint32_t src_offset = ((row + j) * size * 4 + col) * img_data_info.com;
            memcpy(buf + dst_offset, img_data_info.ptr + src_offset, size * img_data_info.com);
            dst_offset += size * img_data_info.com;
        }
        if (auto_blit && img_data_idx == 0) {
            rhi::rhi_instance->update_texture(rhi_texture, buf, size * size * img_data_info.com, i, 0, true);
        }
        else {
            rhi::rhi_instance->update_texture(rhi_texture, buf, size * size * img_data_info.com, img_data_idx, i, false);
        }
    }

    delete [] buf;
    buf = nullptr;
}

void Texture::upload_data_internal() {
    TextureContent& tc = _tex_contents[_active_idx];

    bool need_upload = false;
    for (auto img_data_info : tc.img_data_infos) {
        if (!img_data_info.has_upload) {
            need_upload = true;
            break;
        }
    }

    if (!need_upload) {
        return;
    }

    auto img_data_info = tc.img_data_infos[0];

    rhi::TextureCreateInfo ci;
    ci.format = img_data_info.fmt;
    ci.type = tc.st;
    ci.flags = rhi::TextureCreateFlagBit::ShaderResource | 0;
    ci.width = img_data_info.width;
    ci.height = img_data_info.height;
    ci.depth = img_data_info.depth;
    ci.mip_num = tc.mipmap_level_count;
    ci.layer_num = tc.layer_count;

    bool cubemap_enabled = tc.st == rhi::TextureType::TextureCube || 
            tc.st == rhi::TextureType::TextureCubeArray;

    /*if (cubemap_enabled) {
        ci.layer_num = 6;
        ci.width = (uint32_t) (img_data_info.width / 4);
        ci.height = (uint32_t) (img_data_info.height / 3);
        assert(ci.width == ci.height && ci.depth == 1);
    }

    VkExtent3D extent{
            .width = (uint32_t)img_data_info.width,
            .height = (uint32_t)img_data_info.height,
            .depth = (uint32_t)img_data_info.depth,
    };

    bool cubemap_enabled = tc.st == rhi::TextureType::TextureCube || 
            tc.st == rhi::TextureType::TextureCubeArray;
    if (cubemap_enabled) {
        extent.width = (uint32_t) (img_data_info.width / 4);
        extent.height = (uint32_t) (img_data_info.height / 3);
        assert(extent.width == extent.height && extent.depth == 1);
    }
    uint8_t layer_count = cubemap_enabled ? 6 : tc.layer_count;
    VkImageCreateFlags flags = cubemap_enabled ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

    VkImageType it = VK_IMAGE_TYPE_2D;

    switch(tc.st) {
        case TEXTURE_SAMPLER_1D:
            it = VK_IMAGE_TYPE_1D; break;
        case TEXTURE_SAMPLER_3D:
            it = VK_IMAGE_TYPE_3D; break;
        default:
            break;
    }*/

    rhi::TextureRef rhi_texture = nullptr;

    if (tc.rhi_texture == nullptr || tc.fmt != tc.img_data_infos[0].fmt) {
        /*img = std::make_shared<core::Image>(device, extent,
                                                 img_data_info.fmt,
                                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                 static_cast<VkMemoryPropertyFlagBits>(
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                                                 VK_SAMPLE_COUNT_1_BIT, tc.mipmap_level_count, layer_count,
                                                 VK_IMAGE_TILING_OPTIMAL, flags, it);*/
        rhi_texture = rhi::rhi_instance->create_texture(ci);                                         
        
    }
    else {
        rhi_texture = tc.rhi_texture;
    }
    
    for (uint8_t i = 0; i < tc.img_data_infos.size(); i++) {
        if (tc.img_data_infos[i].has_upload) {
            continue;
        }
        bool auto_blit = false;
        if (tc.layer_count == 1) {
            auto_blit = true;
        }
        if (cubemap_enabled) {
            update_cube_data(tc, rhi_texture, i, auto_blit);
        }
        else {
            update_2d_data(tc, rhi_texture, i, auto_blit);
        }
    }
    
    tc.rhi_texture = rhi_texture;
}


void Texture::upload_data(std::shared_ptr<core::Device> device) {
    if (_tex_contents[_active_idx].has_upload) {
        return;
    }
    
    upload_data_internal();
    
    TextureContent& tc = _tex_contents[_active_idx];
    
    rhi::SampleStateCreateInfo ci;
    tc.rhi_sampler = rhi::rhi_instance->create_sample_state(ci);

    // tc.rhi_sampler = std::make_shared<core::Sampler>(device);
    tc.has_upload = true;
    for (auto& img_data_info : tc.img_data_infos) {
        if (img_data_info.ptr && !img_data_info.reused) {
            stbi_image_free(img_data_info.ptr);
            img_data_info.ptr = nullptr;
        }
        img_data_info.has_upload = true;
    }

}

Texture::~Texture() {
    for (auto content : _tex_contents) {
        for (auto img_data_info : content.img_data_infos) {
            if (img_data_info.ptr) {
                stbi_image_free(img_data_info.ptr);
                img_data_info.ptr = nullptr;
            }
        }
        content.img_data_infos.clear();
    }
    _tex_contents.clear();
}

rhi::TextureRef Texture::get_rhi_texture() {
    return _tex_contents[_active_idx].rhi_texture;
}

rhi::SampleStateRef Texture::get_rhi_sampler() {
    return _tex_contents[_active_idx].rhi_sampler;
}

void Texture::add_content(const std::string& path, uint16_t width, uint16_t height, uint16_t depth, rhi::ColorFormat fmt,
    rhi::TextureType st, uint8_t mipmap_level_count) {
    TextureContent tc;
    tc.mipmap_level_count = mipmap_level_count;
    std::string content = utils::FileHelper().load_content(path.c_str());
    ImageDataInfo img_data_info;
    img_data_info.mipmap_level = 0;
    img_data_info.layer_idx = 0;
    img_data_info.width = width;
    img_data_info.height = height;
    img_data_info.depth = depth;
    img_data_info.fmt = fmt;
    img_data_info.ptr = new unsigned char[content.size()];
    img_data_info.data_len = content.size();
    img_data_info.reused = false;
    img_data_info.has_upload = false;
    memcpy(img_data_info.ptr, content.c_str(), sizeof(char) * content.size());
    
    
    tc.img_data_infos.emplace_back(img_data_info);
    tc.has_upload = false;
    tc.st = st;
    tc.fmt = fmt;
    tc.layer_count = 1;
    _tex_contents.emplace_back(tc);
}

void Texture::add_content(const std::string& path, rhi::TextureType st, uint8_t mipmap_level_count, rhi::ColorFormat fmt) {
    TextureContent tc;
    tc.mipmap_level_count = mipmap_level_count;
    std::string content = utils::FileHelper().load_content(path.c_str());
    ImageDataInfo img_data_info;
    img_data_info.mipmap_level = 0;
    img_data_info.layer_idx = 0;
    img_data_info.depth = 1;
    img_data_info.fmt = fmt;
    img_data_info.reused = false;
    img_data_info.has_upload = false;
    img_data_info.ptr = stbi_load_from_memory((unsigned  char*)content.c_str(), (int)content.size(),
                                    &img_data_info.width, &img_data_info.height, &img_data_info.com, 4);
    assert(img_data_info.ptr != nullptr);
    img_data_info.data_len = img_data_info.width * img_data_info.height * img_data_info.com;
    
    tc.img_data_infos.emplace_back(img_data_info);
    tc.has_upload = false;
    tc.st = st;
    tc.fmt = fmt;
    tc.layer_count = 1;
    _tex_contents.emplace_back(tc);
}

void Texture::add_content(const std::vector<std::string> &mipmap_img_paths, rhi::TextureType st, rhi::ColorFormat fmt) {
    TextureContent tc;
    tc.mipmap_level_count = mipmap_img_paths.size();
    for (uint16_t i = 0; i < mipmap_img_paths.size(); i++) {
        std::string content = utils::FileHelper().load_content(mipmap_img_paths[i].c_str());
        ImageDataInfo img_data_info;
        img_data_info.mipmap_level = i;
        img_data_info.layer_idx = 0;
        img_data_info.depth = 1;
        img_data_info.fmt = fmt;
        img_data_info.reused = false;
        img_data_info.has_upload = false;
        img_data_info.ptr = stbi_load_from_memory((unsigned  char*)content.c_str(), (int)content.size(),
                                    &img_data_info.width, &img_data_info.height, &img_data_info.com, 4);
        img_data_info.data_len = img_data_info.width * img_data_info.height * img_data_info.com;
        
        tc.img_data_infos.emplace_back(img_data_info);
    }
    tc.has_upload = false;
    tc.st = st;
    tc.fmt = fmt;
    tc.layer_count = 1;
    _tex_contents.emplace_back(tc);
}

void Texture::add_hdr_content(const std::string& path, rhi::ColorFormat pixel_fmt, rhi::ColorFormat tex_fmt, uint8_t mipmap_level_count) {
    TextureContent tc;
    tc.mipmap_level_count = mipmap_level_count;
    std::string content = utils::FileHelper().load_content(path.c_str());
    ImageDataInfo img_data_info;
    img_data_info.mipmap_level = 0;
    img_data_info.layer_idx = 0;
    img_data_info.depth = 1;
    img_data_info.reused = false;
    img_data_info.has_upload = false;
    img_data_info.fmt = pixel_fmt;
    uint8_t req_com = 4;
    img_data_info.ptr = (unsigned char*)stbi_loadf_from_memory((unsigned  char*)content.c_str(), (int)content.size(),
                                    &img_data_info.width, &img_data_info.height, &img_data_info.com, req_com);
    img_data_info.data_len = img_data_info.width * img_data_info.height * req_com * sizeof(float);
    
    tc.img_data_infos.emplace_back(img_data_info);
    tc.has_upload = false;
    tc.st = rhi::TextureType::Texture2D;
    tc.fmt = tex_fmt;
    tc.layer_count = 1;
    _tex_contents.emplace_back(tc);
}

SingleLayerTexture::SingleLayerTexture(const std::string& img_path, uint16_t width, uint16_t height, uint16_t depth, rhi::ColorFormat fmt,
                    rhi::TextureType st, uint8_t mipmap_level_count) {
    add_content(img_path, width, height, depth, fmt, st, mipmap_level_count);
    _active_idx = 0;
}

SingleLayerTexture::SingleLayerTexture(const std::string &img_path, rhi::TextureType st,  uint8_t mipmap_level_count) {
    
    if (img_path.find(".hdr") == std::string::npos) {
        add_content(img_path, st, mipmap_level_count);
    }
    else {
        add_hdr_content(img_path, rhi::ColorFormat::R32G32B32A32_SFLOAT , rhi::ColorFormat::R16G16B16A16_SFLOAT);
    }
    
    _active_idx = 0;
}

SingleLayerTexture::SingleLayerTexture(const std::vector<std::string>& mipmap_img_paths, rhi::TextureType st) {
    add_content(mipmap_img_paths, st);
    _active_idx = 0;
}

MultiLayerTexture::MultiLayerTexture(const std::string &dir_path, const std::string &name_template,
                                     uint32_t count, uint8_t mipmap_level_count) {

    char buf[256] = {0};
    std::string path_template = std::string("%s") + name_template;
    for (uint32_t i = 0; i < count; i++) {
        memset(buf, 0, 256);
        sprintf(buf, path_template.c_str(), dir_path.c_str(), i);
        add_content(std::string(buf), rhi::TextureType::Texture2D, mipmap_level_count);
    }
    _active_idx = 0;
}

void MultiLayerTexture::set_active_idx(uint32_t idx) {
    _active_idx = idx;
}

SingleLayerTexture::SingleLayerTexture(int width, int height, int depth, 
    rhi::ColorFormat fmt, rhi::TextureType st,
    uint32_t layer_count, uint32_t mipmap_level_count,
    rhi::SampleStateRef sp, bool reused) {
    TextureContent tc;
    tc.fmt = fmt;
    tc.rhi_texture = nullptr;
    
    tc.has_upload = false;
    tc.layer_count = layer_count;
    tc.mipmap_level_count = mipmap_level_count;
    tc.st = st;
    tc.rhi_sampler = sp;

    for (uint8_t i = 0; i < layer_count; i++) {
        for (uint8_t j = 0; j < mipmap_level_count; j++) {
            ImageDataInfo img_data_info;
            img_data_info.fmt = fmt;
            img_data_info.width = width;
            img_data_info.height = height;
            img_data_info.depth = depth;
            img_data_info.ptr = nullptr;
            img_data_info.reused = reused;
            img_data_info.has_upload = true;
            img_data_info.layer_idx = i;
            img_data_info.mipmap_level = j;
            tc.img_data_infos.emplace_back(img_data_info);
        }
    }
    _tex_contents.emplace_back(tc);
    _active_idx = 0;
}

void SingleLayerTexture::update_content(const unsigned char* data, uint32_t data_len, 
            uint32_t layer, uint32_t mipmap_level, uint8_t com) {
    TextureContent& tc = _tex_contents[0];
    int img_data_info_idx = -1;
    for (uint8_t i = 0; i < tc.img_data_infos.size(); i++) {
        auto img_data_info = tc.img_data_infos[i];
        if (img_data_info.layer_idx != layer) {
            continue;
        }
        if (img_data_info.mipmap_level != mipmap_level) {
            continue;
        }
        img_data_info_idx = i;
    }

    ImageDataInfo& img_data_info = tc.img_data_infos[img_data_info_idx];
    if (img_data_info.ptr == nullptr) {
        img_data_info.ptr = new unsigned char[data_len];
        img_data_info.data_len = data_len;
        img_data_info.com = com;
    }
    else {
        assert(img_data_info.data_len == data_len);
        assert(img_data_info.com == com);
    }
    memcpy(img_data_info.ptr, data, data_len);
    img_data_info.has_upload = false;
    tc.has_upload = false;
}
