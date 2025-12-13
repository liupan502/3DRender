//
// Created by zhida.ji1 on 2022/8/9.
//


#include <algorithm>
#include <functional>
#include <core/core.h>
#include <memory>
#include <string.h>

using namespace zr::core;

// android_app* zr::core::Instance::androidAppCtx = nullptr;

Instance::Instance(const std::string &app_name,
                   const std::unordered_map<const char*, bool>&required_extensions,
                   const std::vector<const char *> &requred_validation_layers, bool headless,
                   uint32_t api_version) {
    try_enable_extensions(required_extensions);
    VkApplicationInfo app_info;
    create_app_info(app_name, api_version, app_info);
#ifdef WIN32
    // _enabled_extensions.emplace_back("VK_EXT_debug_utils");
#endif 

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledExtensionCount = _enabled_extensions.size();
    instance_info.ppEnabledExtensionNames = _enabled_extensions.data();
    instance_info.ppEnabledLayerNames = requred_validation_layers.data();
    instance_info.enabledLayerCount = requred_validation_layers.size();
#ifndef PLATFORM_ANDROID
    //instance_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    
    CALL_VK(vkCreateInstance(&instance_info, nullptr, &_vk_instance));
}

void Instance::fetch_gpus() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(_vk_instance, &count, nullptr);
    std::vector<VkPhysicalDevice> vk_physical_devices(count);
    vkEnumeratePhysicalDevices(_vk_instance, &count, vk_physical_devices.data());

    for (auto& vk_physical_device : vk_physical_devices) {
        _gpus.push_back(std::make_shared<PhysicalDevice>(this, vk_physical_device));
    }

}

bool Instance::try_enable_extensions(
        const std::unordered_map<const char *, bool> &required_extensions) {

    // get available extensions
    uint32_t extension_count;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

    _enabled_extensions.clear();

    for (auto extension : required_extensions) {
        auto extension_name = extension.first;
        auto is_optional = extension.second;
        bool is_find = false;
        for (auto available_extension : available_extensions) {
            if (strcmp(available_extension.extensionName, extension_name) == 0) {
                is_find = true;
                break;
            }
        }
        if (!is_find && !is_optional) {
            return false;
        }
        _enabled_extensions.push_back(extension_name);
    }

    return true;
}

void Instance::create_app_info(const std::string &app_name, uint32_t api_version,
                                            VkApplicationInfo& app_info) {
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.applicationVersion = 0;

    app_info.pApplicationName = app_name.c_str();
    app_info.apiVersion = api_version;
    app_info.engineVersion = 0;
    app_info.pEngineName = app_name.c_str();
}

void Instance::choose_suitable_gpu(VkSurfaceKHR surface) {
    for (auto& gpu : _gpus) {
        if (gpu->is_suitable(surface)) {
            _suitable_gpu = gpu;
            break;
        }
    }
}

void Instance::set_surface(VkSurfaceKHR surface) {
    _vk_surface = surface;
    fetch_gpus();
    choose_suitable_gpu(surface);
}

Instance::~Instance() {
    vkDestroySurfaceKHR(_vk_instance, _vk_surface, nullptr);
    vkDestroyInstance(_vk_instance, nullptr);
}

