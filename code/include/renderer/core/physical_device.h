//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "device.h"
namespace zr {
    namespace core {
        class Instance;
        class PhysicalDevice {
        public:
            PhysicalDevice(Instance* instance, VkPhysicalDevice vk_physical_device);
            virtual ~PhysicalDevice();

            bool map_memory_type_to_idx(uint32_t type_bits,
                                        VkFlags req_mask,
                                        uint32_t* type_idx) const;
            bool is_suitable(VkSurfaceKHR surface) const;

            void ensure_graphic_queue_family();

            std::shared_ptr<Device> create_device();

            const std::vector<const char*>& enabled_extensions() const;

            inline const uint32_t&  get_graphic_queue_family_idx() const { return _graphic_queue_family_idx;};
            inline VkPhysicalDevice get() const { return _vk_physical_device; };
            inline const VkPhysicalDeviceProperties& get_properties() const { return _vk_physical_device_properties;};
            inline Instance* get_instance() const { return _instance; };

            inline const std::vector<VkSurfaceFormatKHR>& get_formats() const { return _formats; };

            inline const VkSurfaceCapabilitiesKHR& get_surface_cap() const { return _surface_cap; };   
        private:
            std::vector<std::shared_ptr<Device>> _devices;
            VkPhysicalDevice _vk_physical_device;
            uint32_t _graphic_queue_family_idx{0};
            Instance* _instance;
            VkPhysicalDeviceProperties _vk_physical_device_properties{};
            std::vector<VkSurfaceFormatKHR> _formats;
            VkSurfaceCapabilitiesKHR _surface_cap;

        };
    }
}