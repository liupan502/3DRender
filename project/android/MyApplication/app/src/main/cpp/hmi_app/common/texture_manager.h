//
// Created by zhida.ji1 on 2022/9/8.
//

#pragma once

#include <scenegraph/scene_graph.h>
#include <memory>

namespace hmi_app{

    enum TextureName{
        TEXTURE_NAME_NONE,
        TEXTURE_NAME_PARK_NO_AVAILABLE,
        TEXTURE_NAME_PARK_AVAILABLE,
        TEXTURE_NAME_PARK_SELECTED,
        TEXTURE_NAME_RADAR_WAVE,
        TEXTURE_NAME_TEST_YELLOW,
        TEXTURE_NAME_TEST_BLUE,
        TEXTURE_NAME_TEST_GREEN,
        TEXTURE_NAME_MAX
    };

    class TextureManager{
    public:
        class Destructor{
        private:
            ~Destructor();
        };
    public:
        static TextureManager* get_instance();
    private:
        static Destructor _destructor;
        static TextureManager* _tex_mgr;

    public:
        std::shared_ptr<zr::sg::Texture> get_tex(TextureName tex_name);
    private:
        TextureManager(){};
        ~TextureManager(){};
        void add_texture(TextureName tex_name);

    private:
        std::map<TextureName, std::shared_ptr<zr::sg::Texture>> _tex_map;
    };
}
