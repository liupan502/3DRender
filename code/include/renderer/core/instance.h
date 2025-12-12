//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

// should be replaced later


namespace zr {
    namespace core {
        class PhysicalDevice;
        class Instance {
        public:
            // static android_app* androidAppCtx;
        public:
            Instance(const std::string& app_name,
                     const std::unordered_map<const char*, bool>& required_extensions = {},
                     const std::vector<const char*>& requred_validation_layers = {},
                     bool headless = false,
                     uint32_t api_version = VK_API_VERSION_1_0);

            void set_surface(VkSurfaceKHR surface);
            inline std::shared_ptr<PhysicalDevice> get_suitable_gpu() { return _suitable_gpu;};
            inline const std::vector<const char*>& enabled_extensions() const { return _enabled_extensions; };
            inline const VkSurfaceCapabilitiesKHR surface_cap() const { return _surface_cap;};
            inline VkInstance get() const { return _vk_instance;};
            virtual ~Instance();
        protected:
            void choose_suitable_gpu(VkSurfaceKHR surface);
            bool try_enable_extensions(const std::unordered_map<const char*, bool>& required_extensions);
            void create_app_info(const std::string& app_name, uint32_t api_version, VkApplicationInfo& app_info);
            void fetch_gpus();
        private:
            VkSurfaceKHR _vk_surface{VK_NULL_HANDLE};
            VkInstance _vk_instance{VK_NULL_HANDLE};
            std::vector<const char*> _enabled_extensions;
            std::vector<std::shared_ptr<PhysicalDevice>> _gpus;
            std::shared_ptr<PhysicalDevice> _suitable_gpu;
            // VkFormat _fmt{VK_FORMAT_UNDEFINED};
            VkSurfaceCapabilitiesKHR _surface_cap;
        };
    }
}
