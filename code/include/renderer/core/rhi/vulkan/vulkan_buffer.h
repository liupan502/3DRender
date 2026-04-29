#pragma once
#include "rhi/rhi_resource.h"
#include <render_context.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace rhi {
namespace vulkan {

class VulkanBuffer : public rhi::Buffer {
public:
    VulkanBuffer(std::shared_ptr<zr::RenderContext> context,
                 const BufferCreateInfo& create_info);

    virtual ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    inline bool is_valid() const { return _is_valid; }
    virtual VkBuffer get() const;
    inline VkDeviceSize get_size() const { return _size; }
    virtual void update(const uint8_t* data, size_t size, size_t offset = 0, bool do_unmap = true);

protected:
    virtual VkBufferCreateInfo get_create_info(VkDeviceSize size,
                                               VkBufferUsageFlagBits usage,
                                               const std::vector<uint32_t>& queue_family_indices);

    virtual bool init_device_memory(VkDeviceSize size);

    VkMemoryRequirements fetch_mem_req(VkBuffer vk_buf);
    bool alloc_mem(VkDeviceSize size, uint32_t memory_type_bits);
    bool bind_mem(VkBuffer vk_buf, uint32_t offset);

    virtual void create_handles(VkDeviceSize size, VkBufferUsageFlagBits usage);

    bool map();
    void unmap();

    static VkBufferUsageFlagBits buffer_usage_to_vk(BufferUsageFlags usage);

protected:
    std::shared_ptr<zr::RenderContext> _context;
    bool _is_valid = false;
    std::vector<VkBuffer> _vk_buffers;
    VkDeviceMemory _vk_device_memory{VK_NULL_HANDLE};
    void* _mapped_addr{nullptr};
    bool _mapped{false};
    VkDeviceSize _size{0};
};

} // namespace vulkan
} // namespace rhi
