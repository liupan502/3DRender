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

        /*class ShaderResource {
        private:
            size_t _id;
            std::string _file_name;
            std::string _source;
        };*/

        class ShaderModule {

        public:
            ShaderModule(std::shared_ptr<Device> _device, const char* file_path, bool use_bin = true, std::vector<std::string> defines = std::vector<std::string>());
            inline VkShaderModule get() const { return _vk_shader_module;};
            virtual ~ShaderModule();
        private:
            // for spv shader
            void load_shader_from_file(const char* file_path);

            // for glsl shader, need compile to spv before using
            void create_shader_from_file(const char* base_dir, const char* file_path, std::vector<std::string> defines);
        private:
            std::shared_ptr<Device> _device;
            VkShaderModule _vk_shader_module{};
            // size_t _id;
            // VkShaderStageFlagBits _stage{};
            // std::string _entry_point;
            // std::vector<uint32_t> _spirvs;


        };
    }
}
