//
// Created by zhida.ji1 on 2022/8/8.
//

#include <core/buffer.h>
#include <core/device.h>
#include <vulkan/vulkan_android.h>
#include <string.h>
#include <memory>
#include <assert.h>

using namespace zr::core;

Buffer::Buffer(std::shared_ptr<Device> device, VkDeviceSize size,
               VkBufferUsageFlagBits usage) : _device(device) {
    _is_valid = true;

    create_handles(size, usage);

    if (_is_valid) {
        init_device_memory(_size);
    }
}

VkBuffer Buffer::get() const {
    return _vk_buffers[0];
}

Buffer::~Buffer() {
    for (uint32_t i = 0; i < _vk_buffers.size(); i++) {
        vkDestroyBuffer(_device->get_device(), _vk_buffers[i], nullptr);
    }
    _vk_buffers.clear();
    vkFreeMemory(_device->get_device(), _vk_device_memory, nullptr);
    
}

void Buffer::create_handles(VkDeviceSize size, VkBufferUsageFlagBits usage) {
    std::vector<uint32_t> queue_family_indices{_device->get_graphic_queue_family_idx()};
    VkBufferCreateInfo ci = get_create_info(size, usage, queue_family_indices);
    VkBuffer vk_buf = VK_NULL_HANDLE;
    VkResult  ret = vkCreateBuffer(_device->get_device(), &ci, nullptr, &vk_buf);
    
    if (ret != VK_SUCCESS) {
        _is_valid = false;
    }
    _vk_buffers.emplace_back(vk_buf);
}

VkBufferCreateInfo Buffer::get_create_info(VkDeviceSize size,
                                   VkBufferUsageFlagBits usage,
                                   const std::vector<uint32_t>& queue_family_indices) {
    VkBufferCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ci.usage = usage;
    ci.size = size;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = queue_family_indices.size();
    ci.pQueueFamilyIndices = queue_family_indices.data();

    return ci;
}

VkMemoryRequirements Buffer::fetch_mem_req(VkBuffer vk_buf) {
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(_device->get_device(), vk_buf, &mem_req);
    return mem_req;
}

bool Buffer::alloc_mem(VkDeviceSize size, uint32_t memoryTypeBits) {
    VkMemoryAllocateInfo allocate_info;
    allocate_info.pNext = nullptr;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = size;
    _device->map_memory_type_to_idx(memoryTypeBits,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_info.memoryTypeIndex);
    VkResult ret = vkAllocateMemory(_device->get_device(), &allocate_info,
                     nullptr, &_vk_device_memory);
    if (ret != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool Buffer::bind_mem(VkBuffer vk_buf, uint32_t offset) {
   VkResult ret = vkBindBufferMemory(_device->get_device(), vk_buf, _vk_device_memory, offset);
    if (ret != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool Buffer::init_device_memory(VkDeviceSize size) {
    VkMemoryRequirements mem_req = fetch_mem_req(_vk_buffers[0]);
    
    _size = mem_req.size;

    if (!alloc_mem(_size, mem_req.memoryTypeBits)) {
        return false;
    }

    if (!bind_mem(_vk_buffers[0], 0)) {
        return false;
    }
   
    return true;
}

bool Buffer::map() {
    if (_mapped) {
        return true;
    }
    VkResult ret = vkMapMemory(_device->get_device(),
                               _vk_device_memory, 0, _size, 0, &_mapped_addr);
    if (ret == VK_SUCCESS) {
        _mapped = true;
    }
    return ret == VK_SUCCESS;
}

void Buffer::unmap() {
    if (!_mapped) {
        return ;
    }
    vkUnmapMemory(_device->get_device(), _vk_device_memory);
    _mapped = false;
    _mapped_addr = nullptr;
}

void Buffer::update(const uint8_t *data, size_t size, size_t offset, bool do_unmap) {
    const float* tmp = (const float*)(data);

    map();
    // std::copy(data, data + size, ((uint8_t*)(_mapped_addr)) + offset);
    memcpy((char*)(_mapped_addr) + offset, data, size);
    if (do_unmap) {
        unmap();
    }
}

UniformBuffer::UniformBuffer(uint32_t binding_idx,
                             uint32_t size, uint32_t element_count) :
                             _element_count(element_count),
                             _binding_idx(binding_idx), _data_size(size){

    
    // _is_valid = true;

    create_handles(size);
}

void UniformBuffer::update(const uint8_t* data, size_t size, 
            size_t offset, bool do_unmap) {
    rhi::rhi_instance->update_buffer(_hw_buffers[_active_element_idx], (void*)data, size, offset);
}

void UniformBuffer::create_handles(uint32_t size) {

    for (uint32_t i = 0; i < _element_count; i++) {
        rhi::BufferCreateInfo ci;
        ci.size = size;
        ci.usage = rhi::BufferUsageFlagBit::UniformBuffer | 0;
        auto buf = rhi::rhi_instance->create_buffer(ci);
        _hw_buffers.emplace_back(buf);
    }
}

rhi::BufferRef UniformBuffer::get() const {
    return _hw_buffers[_active_element_idx];
}
