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
    _physical_device->ensure_graphic_queue_family();
    _device = _physical_device->create_device();
    LOGD("create_device success")
    _cmd_pool = std::make_shared<core::CommandPool>(_device);
    LOGD("CommandPool success")
    _device->set_cmd_pool(_cmd_pool);
    _queue = _device->get_cmd_queue();

    _swap_chain = std::make_shared<core::Swapchain>(_physical_device, _device, vk_surface, swapchain_fmt);
    LOGD("Swapchain success")

    // Create sync objects
    VkSemaphoreCreateInfo semaphore_ci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    CALL_VK(vkCreateSemaphore(_device->get_device(), &semaphore_ci, nullptr, &_vk_semaphore));

    VkFenceCreateInfo fence_ci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    CALL_VK(vkCreateFence(_device->get_device(), &fence_ci, nullptr, &_vk_fence));

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



    required_extensions.insert(std::make_pair(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true));

    // enable validation
    std::vector<const char*> required_validation_layers = {"VK_LAYER_KHRONOS_validation"};

    _instance = std::make_shared<core::Instance>("zr_engine", required_extensions, required_validation_layers);
    // core::Instance::androidAppCtx = app;

    setup_debug_messenger();

    VkSurfaceKHR vk_surface;
    CALL_VK(glfwCreateWindowSurface(_instance->get(), window, nullptr, &vk_surface));
    LOGD("vkCreateAndroidSurfaceKHR success")
    _instance->set_surface(vk_surface);
    _physical_device = _instance->get_suitable_gpu();
    LOGD("get_suitable_gpu success")
    _physical_device->ensure_graphic_queue_family();
    _device = _physical_device->create_device();
    LOGD("create_device success")
    _cmd_pool = std::make_shared<core::CommandPool>(_device);
    LOGD("CommandPool success")
    _device->set_cmd_pool(_cmd_pool);
    _queue = _device->get_cmd_queue();

    _swap_chain = std::make_shared<core::Swapchain>(_device, swapchain_fmt);
    // LOGD("Swapchain success")

    // Create sync objects
    VkSemaphoreCreateInfo semaphore_ci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    CALL_VK(vkCreateSemaphore(_device->get_device(), &semaphore_ci, nullptr, &_vk_semaphore));

    VkFenceCreateInfo fence_ci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    CALL_VK(vkCreateFence(_device->get_device(), &fence_ci, nullptr, &_vk_fence));

    return true;
}
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* /*user_data*/)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOGE("Vulkan: %s", callback_data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOGW("Vulkan: %s", callback_data->pMessage);
    } else {
        LOGD("Vulkan: %s", callback_data->pMessage);
    }
    return VK_FALSE;
}

void RenderContext::setup_debug_messenger()
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(_instance->get(), "vkCreateDebugUtilsMessengerEXT");
    if (!func) return;

    VkDebugUtilsMessengerCreateInfoEXT ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    ci.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci.pfnUserCallback = debug_callback;
    func(_instance->get(), &ci, nullptr, &_debug_messenger);
}

RenderContext::~RenderContext() {
    if (_debug_messenger != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(_instance->get(), "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(_instance->get(), _debug_messenger, nullptr);
    }
    if (_vk_semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(_device->get_device(), _vk_semaphore, nullptr);
    }
    if (_vk_fence != VK_NULL_HANDLE) {
        vkDestroyFence(_device->get_device(), _vk_fence, nullptr);
    }

    _swap_chain = nullptr;

    _queue = nullptr;
    _device->set_cmd_pool(nullptr);

    _cmd_pool = nullptr;

    _device = nullptr;

    _physical_device = nullptr;

    _instance = nullptr;

}

uint32_t RenderContext::acquire_image(VkSemaphore signal_semaphore, uint64_t timeout) {
    VkResult result = vkAcquireNextImageKHR(
        _device->get_device(),
        _swap_chain->get(),
        timeout,
        signal_semaphore,
        VK_NULL_HANDLE,
        &_current_frame_index
    );

    if (result != VK_SUCCESS) {
        // Handle suboptimal or out-of-date errors
        return UINT32_MAX;
    }

    return _current_frame_index;
}

void RenderContext::present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore) {

    // Wait for fence and reset
    auto deviceHandle = this->get_device()->get_device();
    auto fenceHandle = this->get_fence();
    CALL_VK(vkWaitForFences(deviceHandle, 1, &fenceHandle, VK_TRUE, 100000000));
    CALL_VK(vkResetFences(deviceHandle, 1, &fenceHandle));

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;

    if (wait_semaphore != VK_NULL_HANDLE) {
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &wait_semaphore;
    } else {
        present_info.waitSemaphoreCount = 0;
        present_info.pWaitSemaphores = nullptr;
    }

    present_info.swapchainCount = 1;
    auto swapchainHandle = _swap_chain->get();
    present_info.pSwapchains = &swapchainHandle;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(queue, &present_info);
}

void RenderContext::submit(VkQueue queue, VkCommandBuffer cmd_buf, VkSemaphore wait_semaphore,
                          VkPipelineStageFlags wait_stage, VkFence fence) {
    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = wait_semaphore != VK_NULL_HANDLE ? 1u : 0u,
        .pWaitSemaphores = wait_semaphore != VK_NULL_HANDLE ? &wait_semaphore : nullptr,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    vkQueueSubmit(queue, 1, &submit_info, fence);
}
