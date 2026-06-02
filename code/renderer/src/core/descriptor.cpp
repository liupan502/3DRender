//
// Created by zhida.ji1 on 2022/8/9.
//


#include <core/core.h>

#include <scenegraph/components/material.h>
#include <scenegraph/components/skin.h>
#include <scenegraph/components/texture.h>
#include <core/sampler.h>

using namespace zr::core;

extern const unsigned int MAX_UNIFORM_BUFFER_OBJECT_COUNT;

/*DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(_device->get_device(), _vk_desc_pool, nullptr);
}


std::vector<VkDescriptorSet> DescriptorPool::create_vk_desc_sets(uint32_t set_count) {
    std::vector<VkDescriptorSet> sets(set_count);
    std::vector<VkDescriptorSetLayout> vk_desc_layouts(set_count, _layout->get());
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pSetLayouts = vk_desc_layouts.data();
    alloc_info.descriptorSetCount = set_count;
    alloc_info.descriptorPool = _vk_desc_pool;
    
    CALL_VK(vkAllocateDescriptorSets(_device->get_device(), &alloc_info, sets.data()));

    return sets;
}*/

/*DescriptorLayout::~DescriptorLayout() {
    vkDestroyDescriptorSetLayout(_device->get_device(), _vk_desc_set_layout, nullptr);
}*/

void DescriptorLayout::add_binding(uint32_t binding_idx, rhi::DescriptorType desc_type,
                                   uint32_t desc_count, rhi::ShaderStageType stage_flags) {
    
    _binding_infos.emplace_back(rhi::DescriptorBindingInfo({binding_idx, stage_flags, desc_type,  desc_count}));
}

void DescriptorLayout::add_vertex_uniform_binding(uint32_t binding, uint32_t desc_count) {
    add_binding(binding, rhi::DescriptorType::DT_UNIFORM_BUFFER,
                desc_count, rhi::ShaderStageType::SST_VERTEX);
}

void DescriptorLayout::add_fragment_uniform_binding(uint32_t binding, uint32_t desc_count) {
    add_binding(binding, rhi::DescriptorType::DT_UNIFORM_BUFFER,
                desc_count, rhi::ShaderStageType::SST_FRAGMENT);
}

void DescriptorLayout::add_fragment_image_sampler_binding(uint32_t binding, uint32_t desc_count) {
    add_binding(binding, rhi::DescriptorType::DT_SAMPLER_2D,
                desc_count, rhi::ShaderStageType::SST_FRAGMENT);
}

/*std::map<VkDescriptorType, uint32_t> DescriptorLayout::get_types() const {
    std::map<VkDescriptorType, uint32_t> ret;
    for (auto binding : _bindings) {
        if (ret.find(binding.descriptorType) == ret.end()) {
            ret.insert(std::make_pair(binding.descriptorType, 0));
        }
        ret[binding.descriptorType] += binding.descriptorCount;
    }
    return ret;
}*/

DescriptorLayout::DescriptorLayout(PipelineFeature feature)
{

}

DescriptorLayout::DescriptorLayout(){
    // create_layout();
}

DescriptorLayout::DescriptorLayout(const std::vector<rhi::DescriptorBindingInfo>& binding_infos){
    _binding_infos = binding_infos;
}

void DescriptorLayout::init_bindings(PipelineFeature feature) {

}

/*void DescriptorLayout::add_push_constant_range(uint32_t size, uint32_t offset,
                                               VkShaderStageFlagBits stage) {
    VkPushConstantRange range {};
    range.stageFlags = stage;
    range.offset = offset;
    range.size = size;
    _push_constant_ranges.emplace_back(range);
}*/

void DescriptorLayout::init_push_constants(PipelineFeature feature) {

}

void DescriptorLayout::create_layout(PipelineFeature feature) {
    
    init_bindings(feature);
    
    rhi::DescriptorSetLayoutCreateInfo ci;
    ci.binding_infos = _binding_infos;
    _rhi_layout = rhi::rhi_instance->create_descriptor_set_layout(ci);
}

BaseDescriptorLayout::BaseDescriptorLayout(){
    create_layout(PipelineFeature(LightInfo{}, std::shared_ptr<sg::Material>(nullptr), std::vector<std::vector<sg::VertexAttribute>>()));
}

BaseDescriptorLayout::BaseDescriptorLayout(PipelineFeature feature) : DescriptorLayout(feature){
    create_layout(feature);
}

void BaseDescriptorLayout::init_bindings(PipelineFeature feature) {
    add_vertex_uniform_binding(0, MAX_UNIFORM_BUFFER_OBJECT_COUNT);
    add_vertex_uniform_binding(25, MAX_UNIFORM_BUFFER_OBJECT_COUNT);
    add_fragment_image_sampler_binding(10, 1);
    add_fragment_uniform_binding(18, 1);

    try_enable_light(feature);
    try_enable_skin(feature);
}

/*void BaseDescriptorLayout::init_push_constants(PipelineFeature feature) {
    uint32_t size = 0;
    LightInfo light_info = feature.get_light_info();
    if (!light_info.has_light() || !feature.get_material_ability().enable_light) {
        return;
    }

    if (light_info.point_light_count > 0) {
        size += (light_info.point_light_count * sizeof(float) * 3);
    }

    if (light_info.spot_light_count > 0) {
        size += (light_info.spot_light_count * sizeof(float) * 3);
    }
    if (size == 0) {
        return;
    }

    add_push_constant_range(size, 0, VK_SHADER_STAGE_VERTEX_BIT);
}*/

void BaseDescriptorLayout::try_enable_light(PipelineFeature feature) {
    LightInfo light_info = feature.get_light_info();
    if (!feature.get_material_ability().enable_light && !feature.get_light_info().has_light()) {
        return;
    }

    // light vertex info
    add_vertex_uniform_binding(1, 1);

    // light fragment global info
    add_fragment_uniform_binding(14, 1);

    if (light_info.directional_light_count > 0) {
        add_fragment_uniform_binding(13, light_info.directional_light_count);
    }

    if (light_info.spot_light_count > 0) {
        add_fragment_uniform_binding(12, light_info.spot_light_count);
    }

    if (light_info.point_light_count > 0) {
        add_fragment_uniform_binding(11, light_info.point_light_count);
    }

    if (light_info.environment_light_count > 0 &&
            feature.get_material_ability().enable_environment &&
            feature.get_material_ability().enable_light) {
        add_fragment_image_sampler_binding(15, 1);
        add_fragment_image_sampler_binding(16, 1);
        add_fragment_uniform_binding(17, 1);
    }
}

void BaseDescriptorLayout::try_enable_skin(PipelineFeature feature) {
    if (!feature.get_material_ability().enable_skin) {
        return;
    }
    add_vertex_uniform_binding(2, 1);
}

/*void DescriptorPool::create_pool(uint32_t desc_count) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    std::map<VkDescriptorType, uint32_t> types = _layout->get_types();
    for (auto it = types.begin(); it != types.end(); it++) {
        VkDescriptorPoolSize pool_size{};
        pool_size.type = it->first;
        pool_size.descriptorCount = desc_count * it->second;
        pool_sizes.emplace_back(pool_size);
    }
    VkDescriptorPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.maxSets = desc_count;
    ci.pPoolSizes = pool_sizes.data();
    ci.poolSizeCount = pool_sizes.size();
    CALL_VK(vkCreateDescriptorPool(_device->get_device(), &ci, nullptr, &_vk_desc_pool));
    std::vector<VkDescriptorSet> vk_desc_sets = create_vk_desc_sets(desc_count);
    for (auto vk_desc_set : vk_desc_sets) {
        _desc_sets.emplace_back(std::make_shared<DescriptorSet>(_device, vk_desc_set));
    }
}

DescriptorPool::DescriptorPool(std::shared_ptr<Device> device,
                               const std::vector<DescriptorBindingInfo>& binding_infos,
                               uint32_t desc_count) : _device(device) {
    _layout = std::make_shared<DescriptorLayout>(device, binding_infos);
    create_pool(desc_count);
}

DescriptorPool::DescriptorPool(std::shared_ptr<Device> device, std::shared_ptr<DescriptorLayout> layout, uint32_t desc_count) :
    _device(device), _layout(layout){
    create_pool(desc_count);
}*/

void DescriptorSet::update_desc_set_buffer(std::vector<std::shared_ptr<UniformBuffer>> buffers, uint32_t desc_count, uint32_t dst_arr_ele) {
    
    /*if (buffers.size() == 0) {
        return;
    }

    uint16_t idx = _buffer_current_idx;
    for (uint32_t i = 0; i < buffers.size(); i++) {
    
        VkDescriptorBufferInfo buffer_info{
                .buffer = buffers[i]->get(),
                .offset = 0,
                .range = buffers[i]->get_data_size(),
        };
        _buffer_infos[_buffer_current_idx++] = buffer_info;
    
    }

    VkWriteDescriptorSet desc_write_set{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _vk_desc_set,
                .dstBinding = buffers[0]->get_binding_idx(),
                .dstArrayElement = dst_arr_ele,
                .descriptorCount = desc_count,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = (_buffer_infos.data() + idx),
                .pTexelBufferView = nullptr,
    };
    _write_desc_sets.emplace_back(desc_write_set);*/

    for (int i = 0; i < buffers.size(); i++) {
        rhi::rhi_instance->update_desc_buffer(_rhi_desc_set, buffers[i]->get(), buffers[i]->get_binding_idx(), 0, buffers[i]->get_data_size());
    }
}

/*void DescriptorSet::update_desc_set_buffer(std::vector<std::shared_ptr<Buffer>> buffers,
                                            uint32_t binding_idx, uint32_t desc_count, uint32_t arr_len) {
    for (uint32_t i = 0; i < buffers.size(); i++) {
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        for (uint32_t j = 0; j < desc_count; j++) {
            VkDescriptorBufferInfo buffer_info{};
            
            buffer_info.buffer = buffers[i]->get();
            buffer_info.offset = j * buffers[i]->get_size() / arr_len;
            buffer_info.range = buffers[i]->get_size() / arr_len;
            
            buffer_infos.emplace_back(buffer_info);
        }

        VkWriteDescriptorSet desc_write_set{};
        
        desc_write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_write_set.dstSet = _vk_desc_set;
        desc_write_set.dstBinding = binding_idx;
        desc_write_set.dstArrayElement = 0;
        desc_write_set.descriptorCount = desc_count;
        desc_write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc_write_set.pImageInfo = nullptr;
        desc_write_set.pBufferInfo = buffer_infos.data();
        desc_write_set.pTexelBufferView = nullptr;
    

        vkUpdateDescriptorSets(_device->get_device(), 1, &desc_write_set, 0, nullptr);
    }
}*/

void DescriptorSet::update_desc_set_texture(std::shared_ptr<sg::Texture> texture,
                                             uint32_t binding_idx) {
    auto sampler = texture->get_rhi_sampler();
    auto image_view = texture->get_rhi_texture();
    update_desc_set_texture(sampler, image_view, binding_idx);
}

void DescriptorSet::update_desc_set_texture(rhi::SampleStateRef sampler, rhi::TextureRef image_view, 
                                uint32_t binding_idx) {
    /*for (uint32_t i = 0; i < 1; i++) {
        _samplers[_img_current_idx] = sampler;
        VkDescriptorImageInfo& img_info = _img_infos[_img_current_idx++]; 
        
        img_info.sampler = sampler->get();
        img_info.imageView = image_view->get();
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        VkWriteDescriptorSet desc_write_set{};
        
        desc_write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_write_set.dstSet = _vk_desc_set;
        desc_write_set.dstBinding = binding_idx;
        desc_write_set.dstArrayElement = 0;
        desc_write_set.descriptorCount = 1;
        desc_write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc_write_set.pImageInfo = &img_info;
        desc_write_set.pBufferInfo = nullptr;
        desc_write_set.pTexelBufferView = nullptr;
        
        _write_desc_sets.emplace_back(desc_write_set);
    }*/

    rhi::rhi_instance->update_desc_texture(_rhi_desc_set, sampler, image_view, binding_idx);
}

/*void DescriptorSet::update_desc_set_input_attachment(std::shared_ptr<ImageView> img_view, uint32_t binding_idx) {
    VkDescriptorImageInfo& img_info = _img_infos[_img_current_idx++]; 
        
    img_info.sampler = VK_NULL_HANDLE;
    img_info.imageView = img_view->get();
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet desc_write_set{};
        
    desc_write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write_set.dstSet = _vk_desc_set;
    desc_write_set.dstBinding = binding_idx;
    desc_write_set.dstArrayElement = 0;
    desc_write_set.descriptorCount = 1;
    desc_write_set.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    desc_write_set.pImageInfo = &img_info;
    desc_write_set.pBufferInfo = nullptr;
    desc_write_set.pTexelBufferView = nullptr;
    
    _write_desc_sets.emplace_back(desc_write_set);
    // vkUpdateDescriptorSets(_device->get_device(), 1, &desc_write_set, 0, nullptr);
}*/

/*void DescriptorSet::bind(std::shared_ptr<CommandBuffer> cmd_buf,
                         std::shared_ptr<PipelineLayout> pipeline_layout) {
    if (_write_desc_sets.size() > 0) {
        vkUpdateDescriptorSets(_device->get_device(), _write_desc_sets.size(), _write_desc_sets.data(), 0, nullptr);
        _write_desc_sets.clear();
        _img_current_idx = 0;
        _buffer_current_idx = 0;
    }                            

    vkCmdBindDescriptorSets(cmd_buf->get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->get(),
                            0, 1, &_vk_desc_set, 0,
                            nullptr);
}

DescriptorSet::~DescriptorSet() { 

}*/

/*std::vector<std::shared_ptr<DescriptorSet>> DescriptorPool::get_available_desc_sets(
        uint32_t count) {
    std::vector<std::shared_ptr<DescriptorSet>> ret;
    for (auto desc_set : _desc_sets) {
        if (count == 0) {
            break;
        }
        
        if (desc_set.use_count() == 2) {
            ret.emplace_back(desc_set);
            --count;
        }
    }
    return ret;
}*/

