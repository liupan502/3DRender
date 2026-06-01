//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <map>

#include <rhi/rhi.h>
namespace zr {

    namespace sg {
        class Texture;
    };
    namespace core {
        class Device;
        class Buffer;
        class UniformBuffer;

        class CommandBuffer;
        class PipelineLayout;
        class PipelineFeature;
        class ImageView;
        class Sampler;

        
        class DescriptorLayout {

        public:
            DescriptorLayout(PipelineFeature feature);

            DescriptorLayout();

            DescriptorLayout(const std::vector<rhi::DescriptorBindingInfo>& binding_infos);

            // inline VkDescriptorSetLayout get() const { return _vk_desc_set_layout;};
            // std::map<VkDescriptorType, uint32_t> get_types() const;
            virtual ~DescriptorLayout() = default;
        protected:

            void create_layout(PipelineFeature feature);

            virtual void init_bindings(PipelineFeature feature);

            virtual void init_push_constants(PipelineFeature feature);

            void add_vertex_uniform_binding(uint32_t binding, uint32_t desc_count);

            void add_fragment_uniform_binding(uint32_t binding, uint32_t desc_count);

            void add_fragment_image_sampler_binding(uint32_t binding, uint32_t desc_count);

            void add_binding(
                    uint32_t binding_idx,
                    rhi::DescriptorType desc_type,
                    uint32_t desc_count,
                    rhi::ShaderStageType stage_flags);

            // void add_push_constant_range(uint32_t size, uint32_t offset, VkShaderStageFlagBits stage);
        private:
            // std::shared_ptr<Device> _device;
            // bool _is_created{false};
            // std::vector<VkDescriptorSetLayoutBinding> _bindings;
            // std::vector<VkPushConstantRange> _push_constant_ranges;
            // VkDescriptorSetLayout _vk_desc_set_layout{VK_NULL_HANDLE};

            std::vector<rhi::DescriptorBindingInfo> _binding_infos;

        };

        class BaseDescriptorLayout : public DescriptorLayout {

        public:
            BaseDescriptorLayout(PipelineFeature feature);
            BaseDescriptorLayout();
            virtual ~BaseDescriptorLayout() = default;

        protected:
            void try_enable_light(PipelineFeature feature);
            void try_enable_skin(PipelineFeature feature);
        protected:
            virtual void init_bindings(PipelineFeature feature) override;
            virtual void init_push_constants(PipelineFeature feature) override;
        };

        class DescriptorSet {
        public:
            DescriptorSet(std::shared_ptr<Device> device, VkDescriptorSet vk_desc_set) : 
                _device(device), _vk_desc_set(vk_desc_set) {
                _buffer_infos = std::vector<VkDescriptorBufferInfo>(100);
                _img_infos = std::vector<VkDescriptorImageInfo>(20);
                _samplers = std::vector<std::shared_ptr<Sampler>>(20);
            };
            inline VkDescriptorSet get() const { return _vk_desc_set;};

            void update_desc_set_buffer(std::vector<std::shared_ptr<Buffer>> buffers, uint32_t binding_idx,
                                        uint32_t desc_count = 1, uint32_t arr_len = 1);

            void update_desc_set_buffer(std::vector<std::shared_ptr<UniformBuffer>> buffers, uint32_t desc_count = 1, uint32_t dst_arr_ele = 0);

            void update_desc_set_texture(std::shared_ptr<sg::Texture> texture, uint32_t binding_idx);

            void update_desc_set_texture(std::shared_ptr<Sampler> sampler, std::shared_ptr<ImageView> img_view, 
                                uint32_t binding_idx);

            void update_desc_set_input_attachment(std::shared_ptr<ImageView> img_view, uint32_t binding_idx);

            void bind(std::shared_ptr<CommandBuffer> cmd_buf, std::shared_ptr<PipelineLayout> pipeline_layout);

            virtual ~DescriptorSet();


        private:
            std::shared_ptr<Device> _device;
            VkDescriptorSet _vk_desc_set{VK_NULL_HANDLE};
            std::vector<VkWriteDescriptorSet> _write_desc_sets;

            std::vector<VkDescriptorBufferInfo> _buffer_infos;
            uint16_t _buffer_current_idx = 0;
            std::vector<VkDescriptorImageInfo> _img_infos;
            uint16_t _img_current_idx = 0;
            std::vector<std::shared_ptr<Sampler>> _samplers;
        };

        /*class DescriptorPool {
        public:
            DescriptorPool(std::shared_ptr<Device> device, std::shared_ptr<DescriptorLayout> layout, uint32_t size);

            DescriptorPool(std::shared_ptr<Device> device,
                           const std::vector<rhi::DescriptorBindingInfo>& binding_infos, uint32_t size);

            virtual ~DescriptorPool();

            std::vector<std::shared_ptr<DescriptorSet>> get_available_desc_sets(uint32_t count);

            inline VkDescriptorPool get() const { return _vk_desc_pool;};

            inline std::shared_ptr<DescriptorLayout> get_layout() {
                return _layout;
            }

        protected:
            void create_pool(uint32_t desc_count);

        private:
            std::vector<VkDescriptorSet> create_vk_desc_sets(uint32_t count);

        private:
            std::shared_ptr<Device> _device;
            std::shared_ptr<DescriptorLayout> _layout;
            std::vector<std::shared_ptr<DescriptorSet>> _desc_sets;
            VkDescriptorPool _vk_desc_pool {VK_NULL_HANDLE};
        };*/
    }
}
