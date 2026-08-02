
#include <core/physical_device.h>
#include <core/queue.h>
#include <core/instance.h>
#include <core/vk_common.h>

#include <string>

#include <core/device.h>

using namespace zr::core;

bool Device::map_memory_type_to_idx(uint32_t type_bits, VkFlags req_mask, uint32_t *type_idx) const{
    if (!_physical_device) {
        return false;
    }
    return _physical_device->map_memory_type_to_idx(type_bits, req_mask, type_idx);
}

Device::Device(PhysicalDevice *physical_device) : _physical_device(physical_device), _allocator(nullptr){
    assert(physical_device);

    VkDeviceQueueCreateInfo queue_ci = create_queue_create_info(
            _physical_device->get_graphic_queue_family_idx());

    VkDeviceCreateInfo device_ci{};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.ppEnabledLayerNames = nullptr;
    device_ci.enabledExtensionCount = 1;
    const char* extension_name = "VK_KHR_swapchain";
    device_ci.ppEnabledExtensionNames = &(extension_name);
    device_ci.pEnabledFeatures = nullptr;

    CALL_VK(vkCreateDevice(_physical_device->get(), &device_ci, nullptr, &_vk_device));

    VkQueue vk_queue;
    vkGetDeviceQueue(_vk_device, get_graphic_queue_family_idx(), 0, &vk_queue);
    _queue = std::make_shared<Queue>(vk_queue);

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = 0;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorCreateInfo.instance = _physical_device->get_instance()->get();
    allocatorCreateInfo.physicalDevice = _physical_device->get();
    allocatorCreateInfo.device = _vk_device;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorCreateInfo, &_allocator);
}

VkDeviceQueueCreateInfo Device::create_queue_create_info(uint32_t queue_family_idx) const {
    VkDeviceQueueCreateInfo ci{};
    float properties[1] = {1.0f};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    ci.queueFamilyIndex = queue_family_idx;
    ci.flags = 0;
    ci.queueCount = 1;
    ci.pQueuePriorities = properties;

    return ci;
}

const VkPhysicalDeviceProperties& Device::get_physical_properties() const {
    return _physical_device->get_properties();
}

uint32_t  Device::get_graphic_queue_family_idx() const {
    return _physical_device->get_graphic_queue_family_idx();
}

Device::~Device() {
    if (_allocator) {
        vmaDestroyAllocator(_allocator);
    }
    vkDestroyDevice(_vk_device, nullptr);
}