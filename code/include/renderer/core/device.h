//
// Created by zhida.ji1 on 2022/8/8.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <core/command_pool.h>
#include <core/queue.h>
#include <assert.h>
#include <vk_mem_alloc.h>
namespace zr{
    namespace core{
        class PhysicalDevice;
        // class Queue;
        // class CommandPool;
        class DescriptorPool;
        class Device{
        public:
            Device(PhysicalDevice* physical_device);
            const VkDevice& get_device() const { return _vk_device;};

            bool map_memory_type_to_idx(uint32_t type_bits,
                                        VkFlags req_mask,
                                        uint32_t* type_idx) const;

            const VkPhysicalDeviceProperties& get_physical_properties() const;
            uint32_t  get_graphic_queue_family_idx() const;

            inline std::shared_ptr<Queue> get_cmd_queue() { return _queue;};

            inline std::shared_ptr<CommandPool> get_cmd_pool() {return _cmd_pool;};
            inline void set_cmd_pool(std::shared_ptr<CommandPool> cmd_pool) { _cmd_pool = cmd_pool;};

            inline PhysicalDevice* get_gpu() {return _physical_device;};
            
            inline VmaAllocator get_vma_allocator() {return _allocator;};
            virtual ~Device();

        protected:
            VkDeviceQueueCreateInfo create_queue_create_info(uint32_t queue_family_idx) const;

        private:
            PhysicalDevice* _physical_device{nullptr};
            VkDevice _vk_device;
            std::vector<VkExtensionProperties> _extension_properties;
            std::shared_ptr<Queue> _queue;
            std::shared_ptr<CommandPool> _cmd_pool;
            // std::weak_ptr<DescriptorPool> _desc_pool;
            VmaAllocator _allocator;
        };
    }
}
