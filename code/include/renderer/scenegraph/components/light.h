//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once
#include <scenegraph/component.h>
#include <glm/glm.hpp>
#include <vector>
namespace zr {
        namespace core {
            class UniformBuffer;
        }
        namespace sg{
        class Texture;
        enum LightType{
            LIGHT_TYPE_NONE,
            LIGHT_TYPE_SPOT,
            LIGHT_TYPE_DIRECTIONAL,
            LIGHT_TYPE_POINT,
            LIGHT_TYPE_ENVIRONMENT,
            LIGHT_TYPE_MAX
        };

        class Light : public Component{
        public:
            virtual std::type_index get_type() const override;
            Light(Node* node, LightType type) : Component(node),  _type(type){};
            inline LightType get_light_type() const { return _type;};
        private:
            LightType _type{LIGHT_TYPE_NONE};
            // glm::vec3 _color;
        };

        class PointLight : public Light {
        public:
            struct PointLightInfo{
                alignas(16) glm::vec4 color;
                float range_inverse;
            };
        public:
            PointLight(Node* node, LightType type = LIGHT_TYPE_POINT) :
            Light(node, type){
                    _info = PointLightInfo{};
            };

            inline void set_params(float range_inverse, glm::vec4 color) {
                _info = PointLightInfo{color, range_inverse};
            }

            inline PointLightInfo get_light_info() const { return _info;};
        private:
            PointLightInfo _info;
        };

        class SpotLight : public Light{
        public:
            struct SpotLightInfo{
                alignas(16) glm::vec4 color;
                alignas(16) glm::vec3 direction;
                float range_inverse;
                float inner_angle_cos;
                float outer_angle_cos;
            };
        public:
            SpotLight(Node* node,  LightType type = LIGHT_TYPE_SPOT) :
            Light(node, type) {
                    _info = SpotLightInfo{};
            };

            inline void set_params(float outer, float inner, float range_inverse, glm::vec4 color, glm::vec3 direction) {
                _info = SpotLightInfo{color, direction, range_inverse, inner, outer};
            }
            inline SpotLightInfo get_light_info() const { return _info;};
        private:
            SpotLightInfo _info;
        };

        class DirectionalLight : public Light {
        public:
            struct DirectionalLightInfo{
                alignas(16) glm::vec4 color;
                alignas(16) glm::vec3 direction;
            };
        public:
            DirectionalLight(Node* node,LightType type = LIGHT_TYPE_SPOT) : Light(node, type) {
                    _info = DirectionalLightInfo{};
            };
            inline void set_params(glm::vec3 direction, glm::vec4 color) {
                _info = DirectionalLightInfo{color, direction};
            }
            inline DirectionalLightInfo get_light_info() const { return _info;};
        private:
            DirectionalLightInfo _info;
        };

        class EnvironmentLight : public Light {
        public:
            EnvironmentLight(Node* node,LightType type = LIGHT_TYPE_ENVIRONMENT) : Light(node, type) {};
            void set_params(const std::vector<float>& sh_params,
                            const std::vector<std::string>& pre_filtered_img_paths,
                            const std::string& dfg_img_path);
            inline std::shared_ptr<sg::Texture> get_dfg_tex() const { return _dfg_tex; };
            inline std::shared_ptr<sg::Texture> get_prefiltered_tex() const { return _prefiltered_tex; };
            inline const std::vector<glm::vec4>& get_sh_params() const { return _sh_params; };

        protected:
            std::shared_ptr<sg::Texture> _dfg_tex;
            std::shared_ptr<sg::Texture> _prefiltered_tex;
            std::vector<glm::vec4> _sh_params;

        };

        class PhysicalLightAttributes{
        public:
            inline float get_intensity() const { return _intensity;};
            inline void set_intensity(float intensity) { _intensity = intensity; };
        private:
            float _intensity;
        };

        class PhysicalPointLight : public PhysicalLightAttributes, public PointLight {
        public:
            PhysicalPointLight(Node* node) : PointLight(node) { set_intensity(0.0f);};
        };

        class PhysicalSpotLight : public PhysicalLightAttributes, public SpotLight {
        public:
            PhysicalSpotLight(Node* node) : SpotLight(node) { set_intensity(0.0f); };
        };

        class PhysicalDirectionalLight : public PhysicalLightAttributes, public DirectionalLight {
        public:
            PhysicalDirectionalLight(Node* node) : DirectionalLight(node) { set_intensity(0.0f); };
        };

    }
}

