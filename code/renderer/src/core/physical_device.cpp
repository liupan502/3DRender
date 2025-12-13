//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>

using namespace zr::core;

PhysicalDevice::PhysicalDevice(Instance* instance, VkPhysicalDevice vk_physical_device) :
        _vk_physical_device(vk_physical_device), _instance(instance) {
    vkGetPhysicalDeviceProperties(_vk_physical_device, &_vk_physical_device_properties);

    auto surface = instance->get_surface();
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(get(), surface, &_surface_cap);
    
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(get(), surface, &count, nullptr);
     _formats = std::vector<VkSurfaceFormatKHR>(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(get(), surface, &count, _formats.data());
}

bool PhysicalDevice::map_memory_type_to_idx(uint32_t type_bits, VkFlags req_mask,
                                            uint32_t *type_idx) const {
    VkPhysicalDeviceMemoryProperties mem_pro;
    vkGetPhysicalDeviceMemoryProperties(_vk_physical_device, & mem_pro);
    for (int i = 0; i < 32; i++) {
        if ((type_bits & 1) == 1) {
            if ((mem_pro.memoryTypes[i].propertyFlags & req_mask) == req_mask) {
                *type_idx = i;
                return true;
            }
        }
        type_bits >>= 1;
    }
    return false;
}

bool PhysicalDevice::is_suitable(VkSurfaceKHR surface) const {
    if (_vk_physical_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            _vk_physical_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        return false;
    }
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_vk_physical_device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> family_properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(_vk_physical_device, &count, family_properties.data());

    VkBool32 support = VK_FALSE;
    for (uint32_t idx = 0; idx < count; idx++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(_vk_physical_device, idx, surface, &support);
        if (support) {
            return true;
        }
    }

    
    return false;
}

void PhysicalDevice::ensure_graphic_queue_family() {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_vk_physical_device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> family_properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(_vk_physical_device, &count, family_properties.data());
    for (uint32_t idx = 0; idx < count; idx++) {
        if (family_properties[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            _graphic_queue_family_idx = idx;
            break;
        }
    }
}

const std::vector<const char*>& PhysicalDevice::enabled_extensions() const {
    return _instance->enabled_extensions();
}

std::shared_ptr<Device> PhysicalDevice::create_device() {
    std::shared_ptr<Device> device = std::make_shared<Device>(this);
    // _devices.push_back(device);
    return device;
}


PhysicalDevice::~PhysicalDevice() {

}