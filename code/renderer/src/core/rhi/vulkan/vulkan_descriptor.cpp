#include <rhi/vulkan/vulkan_descriptor.h>
#include <core/vk_common.h>
#include <map>
#include <cassert>

namespace rhi {
namespace vulkan {

static VkDescriptorType desc_type_to_vk(DescriptorType t) {
    switch (t) {
        case DescriptorType::DT_SAMPLER_2D:            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::DT_UNIFORM_BUFFER:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::DT_SHADER_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::DT_INPUT_ATTACHMENT:      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:                                       return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
}

static VkShaderStageFlags shader_stage_to_vk(ShaderStageType stage) {
    switch (stage) {
        case ShaderStageType::SST_VERTEX:   return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStageType::SST_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStageType::SST_COMPUTE:  return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                            return VK_SHADER_STAGE_ALL_GRAPHICS;
    }
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, const DescriptorSetLayoutCreateInfo& ci)
    : DescriptorSetLayout(ci), _device(device) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& b : ci.binding_infos) {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = b.binding_idx;
        binding.descriptorCount = b.desc_count;
        binding.descriptorType = desc_type_to_vk(b.desc_type);
        binding.stageFlags = shader_stage_to_vk(b.shader_stage);
        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layout_ci{};
    layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_ci.pBindings = bindings.data();

    CALL_VK(vkCreateDescriptorSetLayout(_device, &layout_ci, nullptr, &_vk_layout));
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    for (auto pool : _vk_pools) {
        vkDestroyDescriptorPool(_device, pool, nullptr);
    }
    _vk_pools.clear();
    vkDestroyDescriptorSetLayout(_device, _vk_layout, nullptr);
}

void VulkanDescriptorSetLayout::create_new_pool() {
    const uint32_t max_sets = 256;

    std::map<VkDescriptorType, uint32_t> type_counts;
    for (const auto& b : _ci.binding_infos) {
        type_counts[desc_type_to_vk(b.desc_type)] += b.desc_count;
    }

    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto& kv : type_counts) {
        VkDescriptorPoolSize ps{};
        ps.type = kv.first;
        ps.descriptorCount = kv.second * max_sets;
        pool_sizes.push_back(ps);
    }

    VkDescriptorPoolCreateInfo pool_ci{};
    pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_ci.maxSets = max_sets;
    pool_ci.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_ci.pPoolSizes = pool_sizes.data();

    VkDescriptorPool pool = VK_NULL_HANDLE;
    CALL_VK(vkCreateDescriptorPool(_device, &pool_ci, nullptr, &pool));
    _vk_pools.push_back(pool);
}

bool VulkanDescriptorSetLayout::try_allocate_from_pool(VkDescriptorPool pool, VkDescriptorSet* out) {
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &_vk_layout;

    VkResult result = vkAllocateDescriptorSets(_device, &alloc_info, out);
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
        return false;
    }
    if (result != VK_SUCCESS) {
        assert(false);
        return false;
    }
    return true;
}

VkDescriptorSet VulkanDescriptorSetLayout::allocate_set() {
    VkDescriptorSet vk_set = VK_NULL_HANDLE;

    for (auto pool : _vk_pools) {
        if (try_allocate_from_pool(pool, &vk_set)) {
            return vk_set;
        }
    }

    create_new_pool();
    if (!_vk_pools.empty() && try_allocate_from_pool(_vk_pools.back(), &vk_set)) {
        return vk_set;
    }

    return VK_NULL_HANDLE;
}

} // namespace vulkan
} // namespace rhi
