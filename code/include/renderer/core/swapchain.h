//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace zr {
    namespace core {
        class Device;
        class Image;
        class ImageView;
        class PhysicalDevice;
        struct SwapchainProperies{
            VkSwapchainKHR old_swapchain;
            uint32_t image_count{3};
            VkExtent2D extent{};
            VkSurfaceFormatKHR surface_format{};
            uint32_t array_layers;
            VkImageUsageFlags img_usage;
            VkSurfaceTransformFlagBitsKHR pre_transform;
            VkCompositeAlphaFlagBitsKHR composite_alpha;
            VkPresentModeKHR present_mode;
        };

        class Swapchain {
        public:
            Swapchain(std::shared_ptr<PhysicalDevice> physical_device, 
                    std::shared_ptr<Device> device, VkSurfaceKHR surface, VkFormat target_format);
            inline VkSwapchainKHR get() { return _vk_swapchain; };
            inline VkFormat get_suitable_format() const { return _suitable_fmt.format;};
            inline const std::vector<std::shared_ptr<ImageView>>& get_display_image_views() const {
                    return _display_image_views;};
            inline const std::vector<std::shared_ptr<Image>>& get_display_images() const {
                return _display_images;
            }
            inline VkExtent2D get_display_size() const { return _display_size; };
            inline uint32_t  len() const { return _display_image_views.size();};

            virtual ~Swapchain();

        private:
            void init_display();
        private:


            // VkSurfaceKHR _vk_surface{VK_NULL_HANDLE};
            VkSwapchainKHR _vk_swapchain{VK_NULL_HANDLE};
            std::vector<VkImage> _vk_images;
            std::vector<VkSurfaceFormatKHR> _vk_surface_formats;
            std::vector<VkPresentModeKHR> _vk_present_modes;
            SwapchainProperies _properies;
            std::shared_ptr<Device> _device;
            // PhysicalDevice* _physical_device;
            std::vector<std::shared_ptr<Image>> _display_images;
            std::vector<std::shared_ptr<ImageView>> _display_image_views;
            VkSurfaceFormatKHR _suitable_fmt{};
            VkExtent2D _display_size{};
        };
    }
}

