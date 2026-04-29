#include <rhi/vulkan/vulkan_buffer.h>
#include <core/device.h>
#include <cstring>
#include <cassert>

namespace rhi {
namespace vulkan {

VkBufferUsageFlagBits VulkanBuffer::buffer_usage_to_vk(BufferUsageFlags usage) {
    VkBufferUsageFlagBits vk_usage = static_cast<VkBufferUsageFlagBits>(0);
    uint32_t flags = static_cast<uint32_t>(usage);

    if (flags & static_cast<uint32_t>(BufferUsageFlags::VertexBuffer))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (flags & static_cast<uint32_t>(BufferUsageFlags::IndexBuffer))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (flags & static_cast<uint32_t>(BufferUsageFlags::UniformBuffer))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    if (flags & static_cast<uint32_t>(BufferUsageFlags::StorageBuffer))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    if (flags & static_cast<uint32_t>(BufferUsageFlags::IndirectBuffer))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    if (flags & static_cast<uint32_t>(BufferUsageFlags::CopySrc))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if (flags & static_cast<uint32_t>(BufferUsageFlags::CopyDst))
        vk_usage = static_cast<VkBufferUsageFlagBits>(vk_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    return vk_usage;
}

VulkanBuffer::VulkanBuffer(std::shared_ptr<zr::RenderContext> context,
                           const BufferCreateInfo& create_info)
    : _context(context) {
    _create_info = create_info;
    _is_valid = true;

    VkBufferUsageFlagBits vk_usage = buffer_usage_to_vk(create_info.usage);
    create_handles(create_info.size, vk_usage);

    if (_is_valid) {
        init_device_memory(_size);
    }
}

VulkanBuffer::~VulkanBuffer() {
    auto device = _context->get_device();
    for (uint32_t i = 0; i < _vk_buffers.size(); i++) {
        vkDestroyBuffer(device->get_device(), _vk_buffers[i], nullptr);
    }
    _vk_buffers.clear();
    vkFreeMemory(device->get_device(), _vk_device_memory, nullptr);
}

VkBuffer VulkanBuffer::get() const {
    return _vk_buffers[0];
}

void VulkanBuffer::create_handles(VkDeviceSize size, VkBufferUsageFlagBits usage) {
    auto device = _context->get_device();
    std::vector<uint32_t> queue_family_indices{device->get_graphic_queue_family_idx()};
    VkBufferCreateInfo ci = get_create_info(size, usage, queue_family_indices);
    VkBuffer vk_buf = VK_NULL_HANDLE;
    VkResult ret = vkCreateBuffer(device->get_device(), &ci, nullptr, &vk_buf);

    if (ret != VK_SUCCESS) {
        _is_valid = false;
    }
    _vk_buffers.emplace_back(vk_buf);
}

VkBufferCreateInfo VulkanBuffer::get_create_info(VkDeviceSize size,
                                                  VkBufferUsageFlagBits usage,
                                                  const std::vector<uint32_t>& queue_family_indices) {
    VkBufferCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ci.usage = usage;
    ci.size = size;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
    ci.pQueueFamilyIndices = queue_family_indices.data();
    return ci;
}

VkMemoryRequirements VulkanBuffer::fetch_mem_req(VkBuffer vk_buf) {
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(_context->get_device()->get_device(), vk_buf, &mem_req);
    return mem_req;
}

bool VulkanBuffer::alloc_mem(VkDeviceSize size, uint32_t memory_type_bits) {
    auto device = _context->get_device();
    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = size;
    device->map_memory_type_to_idx(memory_type_bits,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &allocate_info.memoryTypeIndex);
    VkResult ret = vkAllocateMemory(device->get_device(), &allocate_info,
                                    nullptr, &_vk_device_memory);
    return ret == VK_SUCCESS;
}

bool VulkanBuffer::bind_mem(VkBuffer vk_buf, uint32_t offset) {
    VkResult ret = vkBindBufferMemory(_context->get_device()->get_device(),
                                      vk_buf, _vk_device_memory, offset);
    return ret == VK_SUCCESS;
}

bool VulkanBuffer::init_device_memory(VkDeviceSize size) {
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

bool VulkanBuffer::map() {
    if (_mapped) {
        return true;
    }
    VkResult ret = vkMapMemory(_context->get_device()->get_device(),
                               _vk_device_memory, 0, _size, 0, &_mapped_addr);
    if (ret == VK_SUCCESS) {
        _mapped = true;
    }
    return ret == VK_SUCCESS;
}

void VulkanBuffer::unmap() {
    if (!_mapped) {
        return;
    }
    vkUnmapMemory(_context->get_device()->get_device(), _vk_device_memory);
    _mapped = false;
    _mapped_addr = nullptr;
}

void VulkanBuffer::update(const uint8_t* data, size_t size, size_t offset, bool do_unmap) {
    map();
    memcpy(static_cast<char*>(_mapped_addr) + offset, data, size);
    if (do_unmap) {
        unmap();
    }
}

} // namespace vulkan
} // namespace rhi
