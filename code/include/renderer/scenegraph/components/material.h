//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once
#include <vector>
#include <map>
#include <memory>
#include <string.h>
#include <scenegraph/component.h>
#include "texture.h"
#include "glm/glm.hpp"
#include "core/buffer.h"
#include <json/json.h>

namespace zr
{
    namespace core
    {
        class UniformBuffer;
    }

    namespace sg
    {

        class Material : public Component
        {
        public:
            struct PbrParams
            {
                float roughness;
                float ior;
                float metallic;
                float clear_coat;
                float clear_coat_roughness;
                // float ambient_occlusion;
                // alignas(16) glm::vec3 emissive;
                PbrParams()
                {
                    roughness = 0.5;
                    ior = 1.5;
                    metallic = 0.5;
                    clear_coat = 0.0;
                    clear_coat_roughness = 0.5;
                }


           
           
            };

            struct FragmentUniformBufferObject
            {
                glm::vec4 emissive_color;
                glm::vec4 jitter_info;
            };

            struct MaterialAbility
            {
                uint32_t enable_blend : 1;
                uint32_t enable_texture : 1;
                uint32_t enable_color : 1;
                uint32_t enable_shadow : 1;
                uint32_t enable_light : 1;
                uint32_t enable_normal_map : 1;
                uint32_t enable_specular : 1;
                uint32_t enable_skin : 1;
                uint32_t enable_pbr : 1;
                uint32_t enable_environment : 1;
                uint32_t enable_transparent : 1;
                uint32_t cull_mode : 2;
                uint32_t enable_depth : 1;

                uint32_t get_val() const
                {
                    uint32_t val = 0;

                    val = (val << 1) | (enable_blend);       // 1
                    val = (val << 1) | (enable_texture);     // 2
                    val = (val << 1) | (enable_color);       // 3
                    val = (val << 1) | (enable_shadow);      // 4
                    val = (val << 1) | (enable_light);       // 5
                    val = (val << 1) | (enable_normal_map);  // 6
                    val = (val << 1) | (enable_specular);    // 7
                    val = (val << 1) | (enable_skin);        // 8
                    val = (val << 1) | (enable_pbr);         // 9
                    val = (val << 1) | (enable_environment); // 10
                    val = (val << 1) | (enable_transparent); // 11
                    val = (val << 2) | (cull_mode);          // 13
                    val = (val << 1) | (enable_depth);       // 14
                    return val;
                }

                void reset()
                {
                    memset(this, 0, sizeof(MaterialAbility));
                }
            };

        public:
            Material(Node *node);
            virtual std::type_index get_type() const override;
            void update_texture(TextureType type, std::shared_ptr<Texture> tex);
            virtual void upload_data(std::shared_ptr<core::Device> device) override;
            virtual void update() override;
            std::shared_ptr<Texture> get_tex(TextureType type);
            void set_light_enabled(bool enabled);
            bool get_light_enabled() const;

            void set_color_enabled(bool enabled);
            bool get_color_enabled() const;

            bool get_skin_enabled() const;
            void set_skin_enabled(bool enabled);

            bool get_transparent_enabled() const;
            void set_transparent_enabled(bool enabled);

            bool get_blend_enabled() const;
            void set_blend_enabled(bool enabled);

            bool get_depth_enabled() const;
            void set_depth_enabled(bool enabled);

            int get_cull_mode() const;

            /*
                0 for cull none
                1 for cull front
                2 for cull back
                3 for cull front and back
            */
            void set_cull_mode(int cull_mode);

            inline MaterialAbility get_ability() const { return _ability; };

            inline std::shared_ptr<zr::core::UniformBuffer> get_pbr_params_buf() { return _pbr_params_buf; };
            inline PbrParams get_pbr_params() const { return _pbr_params; };
            void set_pbr_params(PbrParams params);
            inline PbrParams &get_pbr_params() { return _pbr_params; };

            inline void set_is_jitter(bool is_jitter) { 
                _frag_uniform_buf_obj.jitter_info.x = is_jitter ? 1.0 : 0.0; 
            };
            inline bool is_jitter() const { 
                return _frag_uniform_buf_obj.jitter_info.x > 0.0; 
            };

            void set_environment_enabled(bool enabled);
            bool get_environment_enabled();

            inline std::shared_ptr<core::UniformBuffer> get_fragment_uniform_buf() const
            {
                return _material_frag_uniform_buf;
            }

            inline void set_emissive_color(glm::vec4 emissive_color)
            {
                _frag_uniform_buf_obj.emissive_color = emissive_color;
            }

            inline glm::vec4 get_emissive_color() const
            {
                return _frag_uniform_buf_obj.emissive_color;
            }

            bool load(const std::string &path);
             
            bool save(const std::string &path);


            Json::Value tojson();
             
            void loadjson(Json::Value& json);

        private:
            std::map<TextureType, std::shared_ptr<Texture>> _texs;
            MaterialAbility _ability;
            PbrParams _pbr_params;
            std::shared_ptr<zr::core::UniformBuffer> _pbr_params_buf;

            std::shared_ptr<core::UniformBuffer> _material_frag_uniform_buf;

            // [0][1][2] for color , [4] for strength
            // glm::vec4 _emissive_color;
            FragmentUniformBufferObject _frag_uniform_buf_obj;
            
        };

    }
}
