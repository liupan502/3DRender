#pragma once

#include <rhi/rhi_resource.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace rhi {
    namespace vulkan {
        class VulkanDescriptorSet : public DescriptorSet {
        public:
            VulkanDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout, VkDescriptorSet vk_desc_set) :
                DescriptorSet(layout), _vk_desc_set(vk_desc_set) {};

            inline VkDescriptorSet get() const { return _vk_desc_set; }

        private:
            VkDescriptorSet _vk_desc_set{VK_NULL_HANDLE};
        };

        class VulkanDescriptorSetLayout : public DescriptorSetLayout {
        public:
            VulkanDescriptorSetLayout(VkDevice device, const DescriptorSetLayoutCreateInfo& ci);
            virtual ~VulkanDescriptorSetLayout();

            VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
            VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

            inline const DescriptorSetLayoutCreateInfo& get_ci() const { return _ci; }
            inline VkDescriptorSetLayout get() const { return _vk_layout; }

            VkDescriptorSet allocate_set();

        private:
            bool try_allocate_from_pool(VkDescriptorPool pool, VkDescriptorSet* out);
            void create_new_pool();

        private:
            VkDevice _device{VK_NULL_HANDLE};
            VkDescriptorSetLayout _vk_layout{VK_NULL_HANDLE};
            std::vector<VkDescriptorPool> _vk_pools;
        };
    };

};
