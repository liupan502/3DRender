#pragma once

#include <rhi/rhi_resource.h>

namespace rhi {
    namespace vulkan {
        class VulkanDescriptorSet : public DescriptorSet{
            public:
            VulkanDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout) :
                DescriptorSet(layout) {};
        };

        class VulkanDescriptorSetLayout : public DescriptorSetLayout{
            public:
            VulkanDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& ci) : 
              DescriptorSetLayout(ci) {};  
        };
    };

};