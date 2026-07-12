//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>

using namespace zr::core;

Swapchain::Swapchain(std::shared_ptr<Device> device, VkFormat target_format) :
    _device(device){
    auto gpu = _device->get_gpu(); 
    
    std::vector<VkSurfaceFormatKHR> formats = gpu->get_formats();
    for (auto fmt : formats) {
        if (fmt.format == target_format) {
            _suitable_fmt = fmt;
            break;
        }
    }

    // assert(_suitable_fmt.format == VK_FORMAT_R8G8B8A8_UNORM);
    VkSurfaceCapabilitiesKHR surface_cap = gpu->get_surface_cap();
    _display_size = surface_cap.currentExtent;

    VkSwapchainCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = gpu->get_instance()->get_surface();
    ci.queueFamilyIndexCount = 1;
    ci.pQueueFamilyIndices = &gpu->get_graphic_queue_family_idx();
    ci.imageExtent = _display_size;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.imageFormat = _suitable_fmt.format;
    ci.imageColorSpace = _suitable_fmt.colorSpace;
    ci.imageArrayLayers = 1;
    ci.clipped = VK_FALSE;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.minImageCount = surface_cap.minImageCount + 1;
    ci.oldSwapchain = VK_NULL_HANDLE;
    ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    CALL_VK(vkCreateSwapchainKHR(_device->get_device(), &ci, nullptr, &_vk_swapchain));

    init_display();
}

void Swapchain::init_display() {
    _display_image_views.clear();
    _display_images.clear();
    uint32_t count = 0;
    CALL_VK(vkGetSwapchainImagesKHR(_device->get_device(), _vk_swapchain, &count, nullptr));
    
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(_device->get_device(), _vk_swapchain, &count, images.data());
    for (auto& image : images) {
        _display_images.push_back(std::make_shared<Image>(_device, image));
    }

    for (auto& display_image : _display_images) {
        _display_image_views.push_back(std::make_shared<ImageView>(_device,
                display_image,  _suitable_fmt.format, VK_IMAGE_VIEW_TYPE_2D));
    }
}

Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(_device->get_device(), _vk_swapchain, nullptr);
}
