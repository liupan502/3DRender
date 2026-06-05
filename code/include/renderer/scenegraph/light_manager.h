//
// Created by zhida.ji1 on 2022/9/26.
//

#pragma once
#include "node.h"
#include <memory>
#include <scenegraph/components/light.h>
#include <scenegraph/components/camera.h>
#include <scenegraph/components/material.h>

#define MAX_DIRECTIOANL_LIGHT_NUM 5
#define MAX_SPOT_LIGHT_NUM 3
#define MAX_POINT_LIGHT_NUM 3

namespace zr{
    namespace core {
        class Buffer;
        struct LightInfo;
        class Device;
    }
    namespace sg{
        class Texture;
        class LightManager{
        public:
            LightManager() = default;
            core::LightInfo get_light_info();
            void upload();

            void set_light_nodes(LightType light_type, const std::vector<std::shared_ptr<Node>> light_nodes);
            void update(glm::mat4 view_mat);
            void update_light_vertex_info(std::shared_ptr<sg::Camera> camera);

            std::shared_ptr<Texture> get_dfg_tex() const;
            std::shared_ptr<Texture> get_prefiltered_tex() const;


            inline std::vector<std::shared_ptr<core::UniformBuffer>> get_directional_light_buffers() { return _directional_light_buffers;};
            inline std::shared_ptr<core::UniformBuffer> get_point_light_buffer() { return _point_light_infos_buffer;};
            inline std::shared_ptr<core::UniformBuffer> get_spot_light_buffer() { return _spot_light_infos_buffer;};
            inline std::shared_ptr<core::UniformBuffer> get_light_vertex_buffer() { return _light_vertex_info_buffer;};
            inline std::shared_ptr<core::UniformBuffer> get_sh_buf() { return _sh_buf; };
        private:
            void update_directional(glm::mat4 view_mat);
            void update_spot(glm::mat4 view_mat);
            void update_point(glm::mat4 view_mat);
            void update_environment();
        private:
            std::vector<std::shared_ptr<Node>> _directional_light_nodes;
            std::vector<std::shared_ptr<Node>> _spot_light_nodes;
            std::vector<std::shared_ptr<Node>> _point_light_nodes;
            std::vector<std::shared_ptr<Node>> _environment_light_nodes;

            std::vector<std::shared_ptr<core::UniformBuffer>> _directional_light_buffers;
            std::shared_ptr<core::UniformBuffer> _spot_light_infos_buffer;
            std::shared_ptr<core::UniformBuffer> _point_light_infos_buffer;

            std::shared_ptr<core::UniformBuffer> _light_vertex_info_buffer;
            std::shared_ptr<core::UniformBuffer> _sh_buf;
        };
    }
}
