//
// Created by zhida.ji1 on 2022/8/8.
//


#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <memory>
#include <vector>
#include "vulkan_resource.h"
namespace zr {
    namespace core {
        class Device;
        class Buffer {
        public:
            Buffer() = default;

            Buffer(std::shared_ptr<Device> device,
                   VkDeviceSize size, 
                   VkBufferUsageFlagBits usage);
            virtual ~Buffer();

            inline bool is_valid() const { return _is_valid; };
            virtual VkBuffer get() const; 
            inline VkDeviceSize  get_size() const { return _size;};
            virtual void update(const uint8_t* data, size_t size, size_t offset = 0, bool do_unmap = true);

        protected:
            virtual VkBufferCreateInfo get_create_info(VkDeviceSize size,
                VkBufferUsageFlagBits usage,
                const std::vector<uint32_t>& queue_family_indices);

            virtual bool init_device_memory(VkDeviceSize size);

            VkMemoryRequirements fetch_mem_req(VkBuffer vk_buf);

            bool alloc_mem(VkDeviceSize device_size, uint32_t bits);

            bool bind_mem(VkBuffer vk_buf, uint32_t offset);

            virtual void create_handles(VkDeviceSize size, VkBufferUsageFlagBits usage);

            bool map();
            void unmap();

        protected:
            std::shared_ptr<Device> _device;
            bool _is_valid = false;
            std::vector<VkBuffer> _vk_buffers;
            VkDeviceMemory _vk_device_memory;
            void* _mapped_addr{nullptr};
            bool _mapped{false};
            VkDeviceSize _size{0};
            
        };

        class UniformBuffer : public Buffer{
        public:
            // UniformBuffer() = default;
            UniformBuffer(uint32_t  binding_idx, std::shared_ptr<Device> device, 
                    VkDeviceSize size, uint32_t element_count = 1);
            inline uint32_t get_binding_idx() const { return _binding_idx;};
            
            inline uint32_t get_data_size() const { return _data_size; };

            inline void set_active_element(uint32_t idx) { _active_element_idx = idx;};

            virtual void update(const uint8_t* data, size_t size, size_t offset = 0, bool do_unmap = true) override;

            virtual VkBuffer get() const override; 
        protected:
            virtual bool init_device_memory(VkDeviceSize size) override;

            virtual void create_handles(VkDeviceSize size, VkBufferUsageFlagBits usage) override;

        private:
            uint32_t _binding_idx{};
            
            uint32_t _data_size;

            uint32_t _active_element_idx{0};

            uint32_t _element_count{1};
        };
    }
}




