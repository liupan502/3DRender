//
// Created by zhida.ji1 on 2022/9/8.
//

#include "texture_manager.h"

using namespace hmi_app;

TextureManager* TextureManager::_tex_mgr = nullptr;

TextureManager::Destructor::~Destructor() {
    if (_tex_mgr != nullptr) {
        delete _tex_mgr;
        _tex_mgr = nullptr;
    }
}

TextureManager* TextureManager::get_instance() {
    if (!_tex_mgr) {
        _tex_mgr = new TextureManager();
    }
    return _tex_mgr;
}

std::shared_ptr<zr::sg::Texture> TextureManager::get_tex(TextureName tex_name) {
    if (_tex_map.find(tex_name) == _tex_map.end()) {
        add_texture(tex_name);
    }
    return _tex_map[tex_name];
}

void TextureManager::add_texture(TextureName tex_name) {
    std::string tex_path = "images/green.png";
    switch (tex_name) {
        case TEXTURE_NAME_PARK_AVAILABLE:
            tex_path = "images/park_available.png"; break;
        case TEXTURE_NAME_PARK_NO_AVAILABLE:
            tex_path = "images/park_no_available.png"; break;
        case TEXTURE_NAME_PARK_SELECTED:
            tex_path = "images/park_slect.png"; break;
        case TEXTURE_NAME_RADAR_WAVE:
            tex_path = "images/ic_radarwave.png"; break;
        case TEXTURE_NAME_TEST_YELLOW:
            tex_path = "images/yellow.png"; break;
        case TEXTURE_NAME_TEST_BLUE:
            tex_path = "images/blue.png"; break;
        case TEXTURE_NAME_TEST_GREEN:
            tex_path = "images/green.png"; break;
        default:
            break;
    }
    _tex_map.insert(std::make_pair(tex_name, std::make_shared<zr::sg::SingleLayerTexture>(tex_path.c_str())));
}
