//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <scenegraph/components/material.h>
#include <scenegraph/components/mesh.h>

#include <rhi/rhi.h>
#include <rhi/vulkan/vulkan_pipeline.h>
#include <glm/glm.hpp>


namespace zr {
    namespace sg{
        class Material;
    }
    namespace core {
        class Device;
        class DescriptorSet;
        class FgRenderPass;

        class ShaderModule;

        class CommandBuffer;

        class ShaderResource;

        class DescriptorLayout;

        class DescriptorPool;

        class PipelineLayout;

        struct LightInfo {
            // max for 5
            uint32_t directional_light_count{0};

            // max for 3
            uint32_t point_light_count{0};

            // max for 3
            uint32_t spot_light_count{0};

            uint32_t environment_light_count;
            uint16_t get_val();
            bool has_light();
        };

        class PipelineFeature{

        public:
            class PipelineFeatureComp{
            public:
                bool operator() (const PipelineFeature& lhs, const PipelineFeature& rhs) const;
            };

        public:

            PipelineFeature(LightInfo light_info, std::shared_ptr<sg::Material> material,
                            std::vector<std::vector<sg::VertexAttribute>> vtx_attrs);

            inline sg::Material::MaterialAbility get_material_ability() { return _material_ability;};
            inline LightInfo get_light_info() const { return _light_info;};
            inline std::vector<std::vector<sg::VertexAttribute>> get_attrs() const { return _vtx_attrs;};
            uint64_t get_val();

            std::vector<std::string> get_defines() const;

        private:
            LightInfo _light_info;
            sg::Material::MaterialAbility _material_ability;
            std::vector<std::vector<sg::VertexAttribute>> _vtx_attrs;
        };
        
        class Pipeline {
        public:

            Pipeline(PipelineFeature feature);

            bool create(std::shared_ptr<FgRenderPass> fg_render_pass, uint32_t subpass_idx,
                        std::shared_ptr<DescriptorLayout> desc_layout);

            inline VkPipeline get() const { return _rhi_pipeline ? std::static_pointer_cast<rhi::vulkan::VulkanGraphicsPipeline>(_rhi_pipeline)->get() : VK_NULL_HANDLE; };

            inline std::shared_ptr<PipelineLayout> get_pipeline_layout() { return _layout;};

            std::shared_ptr<DescriptorSet> get_available_desc_set();

            inline std::shared_ptr<DescriptorPool> get_desc_pool() { return _desc_pool;};

            inline std::shared_ptr<Device> get_device() { return _device;};

            void bind(std::shared_ptr<CommandBuffer> cmd_buf);

            virtual ~Pipeline();

        protected:
            virtual void create_input_assembly_state();

            virtual void create_depth_stencil_state();

            virtual void create_rasterization_state();

            virtual void create_color_blend_state();

            virtual void create_multisample_state(rhi::SampleCount sample_count);

            virtual void create_viewport_state(glm::vec2 display_size);

            virtual void create_vtx_input_state();

            // virtual void create_tess_state();

            virtual void create_pipeline_cache();

            virtual void create_shader_stage() {};

            void create_shader_stage(const std::map<VkShaderStageFlagBits, std::string>& shader_map);

            virtual void create_pipeline_layout() {};

            virtual void create_dynamic_states();

        protected:
            VkPipelineInputAssemblyStateCreateInfo _input_assembly_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineDepthStencilStateCreateInfo _depth_stencil_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineRasterizationStateCreateInfo _rasterization_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineColorBlendStateCreateInfo _color_blend_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineMultisampleStateCreateInfo _multisample_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineViewportStateCreateInfo _viewport_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineVertexInputStateCreateInfo _vtx_input_state{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineShaderStageCreateInfo _shader_stage_ci{.sType = VK_STRUCTURE_TYPE_MAX_ENUM};
            VkPipelineCache _vk_pipeline_cache{VK_NULL_HANDLE};
            std::vector<VkVertexInputBindingDescription> _vk_input_binding_descs;
            VkRect2D _vk_scissor{};
            VkViewport _vk_viewport{};
            std::vector<VkPipelineColorBlendAttachmentState> _vk_pipeline_color_blend_states;
            VkSampleMask _vk_sample_mask{~0u};

            std::vector<VkDynamicState> _dynamic_states;
            VkPipelineDynamicStateCreateInfo _dynamic_state_info;

            std::vector<VkVertexInputAttributeDescription> _vk_input_attri_descs;
            std::vector<std::shared_ptr<ShaderModule>> _shader_modules{};
            std::vector<VkPipelineShaderStageCreateInfo> _shader_cis{};
            std::shared_ptr<PipelineLayout> _layout;
            std::shared_ptr<DescriptorLayout> _desc_layout;
            std::shared_ptr<DescriptorPool> _desc_pool;
            std::shared_ptr<Device> _device;
            PipelineFeature _feature;


            rhi::GraphicsPipelineCreateInfo _ci;

            rhi::GraphicsPipelineRef _rhi_pipeline;
        private:

            VkPipeline _vk_pipeline{VK_NULL_HANDLE};

            // PipelineState state;




        };

        class BasePipeline : public Pipeline {
        public:
            BasePipeline(std::shared_ptr<FgRenderPass> render_pass, 
                         uint32_t subpass_idx, PipelineFeature feature);

            virtual ~BasePipeline() = default;
        protected:
            virtual void create_vtx_input_state() override;
            virtual void create_shader_stage() override;
            virtual void create_pipeline_layout() override;
        private:
            uint32_t _stride{5};
        };

        class PipelineLayout {
        public:
            PipelineLayout(std::shared_ptr<Device> device, std::shared_ptr<DescriptorLayout> desc_set_layout);
            inline VkPipelineLayout get() const { return _vk_pipeline_layout;};
            virtual ~PipelineLayout();
        private:

            std::shared_ptr<Device> _device;
            VkPipelineLayout _vk_pipeline_layout{VK_NULL_HANDLE};
            // std::vector<ShaderModule*> _shader_modules;
            // std::unordered_map<std::string, ShaderResource> _shader_resources;
            // std::unordered_map<uint32_t, std::vector<ShaderResource>> _shader_sets;
            // std::vector<DescriptorSetLayout *> _descriptor_set_layouts;
        };
    }
}
