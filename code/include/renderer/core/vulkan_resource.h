//
// Created by root on 2022/8/8.
//

#pragma once

#include <memory>

namespace zr{
    namespace core{
        class Device;
        class VulkanResource {
        public:
            VulkanResource() = default;
            VulkanResource(Device* device) : _device(device) {};

            Device* device();
        private:
            Device* _device;
        };
    }
}
