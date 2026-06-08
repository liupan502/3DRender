//
// Created by zhida.ji1 on 2022/8/9.
//

#pragma once
#include <vulkan/vulkan.h>
#include <string>
// should be replaced later
// #include <android_native_app_glue.h>


namespace zr {
    namespace core {
        class Device;

        class ShaderModule {

        public:
            ShaderModule(std::shared_ptr<Device> _device, const char* file_path);
            inline VkShaderModule get() const { return _vk_shader_module;};
            virtual ~ShaderModule();
        private:
            void load_shader_from_file(const char* file_path);
        private:
            std::shared_ptr<Device> _device;
            VkShaderModule _vk_shader_module{};

        };
    }
}