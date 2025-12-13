//
// Created by zhida.ji1 on 2022/8/15.
//

#include <render_context.h>
#include <core/instance.h>
#include <core/physical_device.h>
#include <vulkan/vulkan_android.h>
#include <core/vk_common.h>
#include <core/command_pool.h>
#include <core/swapchain.h>
#include <core/descriptor.h>
#include <core/pipeline.h>
#include <utils/log.h>
using namespace zr;

#ifdef PLATFORM_ANDROID
bool RenderContext::init(AAssetManager* asset_mgr, ANativeWindow* window, VkFormat swapchain_fmt) {
    std::unordered_map<const char*, bool> required_extensions;
    required_extensions.insert(std::make_pair("VK_KHR_surface", true));
    required_extensions.insert(std::make_pair("VK_KHR_android_surface", true));
    _instance = std::make_shared<core::Instance>("zr_engine", required_extensions);
    // core::Instance::androidAppCtx = app;

    VkSurfaceKHR vk_surface;
    // create surface
    VkAndroidSurfaceCreateInfoKHR surface_create_info{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = window
    };
    CALL_VK(vkCreateAndroidSurfaceKHR(_instance->get(), &surface_create_info, nullptr, &vk_surface));
    LOGD("vkCreateAndroidSurfaceKHR success")
    _instance->set_surface(vk_surface);
    _physical_device = _instance->get_suitable_gpu();
    LOGD("get_suitable_gpu success")
    _device = _physical_device->create_device();
    LOGD("create_device success")
    _cmd_pool = std::make_shared<core::CommandPool>(_device);
    LOGD("CommandPool success")
    _device->set_cmd_pool(_cmd_pool);
    _queue = _device->get_cmd_queue();

    _swap_chain = std::make_shared<core::Swapchain>(_physical_device, _device, vk_surface, swapchain_fmt);
    LOGD("Swapchain success")

    return true;
}

#elif  PLATFORM_GLFW
bool RenderContext::init(GLFWwindow* window, VkFormat swapchain_fmt) {
    std::unordered_map<const char*, bool> required_extensions;

    const char** glfwExtensions;
    uint32_t glfwExtensionCount = 0;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        required_extensions.insert(std::make_pair(glfwExtensions[i], true));
    }
    // required_extensions.insert(std::make_pair("VK_KHR_portability_enumeration",true));
    // VK_KHR_portability_subset
    // required_extensions.insert(std::make_pair("VK_KHR_portability_subset",true));



    // enable validation
    // std::vector<const char*> required_validation_layers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> required_validation_layers = {};

    _instance = std::make_shared<core::Instance>("zr_engine", required_extensions, required_validation_layers);
    // core::Instance::androidAppCtx = app;

    VkSurfaceKHR vk_surface;
    CALL_VK(glfwCreateWindowSurface(_instance->get(), window, nullptr, &vk_surface));
    LOGD("vkCreateAndroidSurfaceKHR success")
    _instance->set_surface(vk_surface);
    _physical_device = _instance->get_suitable_gpu();
    LOGD("get_suitable_gpu success")
    _device = _physical_device->create_device();
    LOGD("create_device success")
    _cmd_pool = std::make_shared<core::CommandPool>(_device);
    LOGD("CommandPool success")
    _device->set_cmd_pool(_cmd_pool);
    _queue = _device->get_cmd_queue();

    _swap_chain = std::make_shared<core::Swapchain>(_device, swapchain_fmt);
    // LOGD("Swapchain success")

    return true;
}
#endif

RenderContext::~RenderContext() {
    _swap_chain = nullptr;
    
    _queue = nullptr;
    _device->set_cmd_pool(nullptr);
    
    _cmd_pool = nullptr;
    
    _device = nullptr;
    
    _physical_device = nullptr;
    
    _instance = nullptr;

}
