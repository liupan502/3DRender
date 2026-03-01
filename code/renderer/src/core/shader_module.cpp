//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>
#include <utils/file_helper.h>
#include <utils/code_generator.h>
#include <utils/log.h>
#include <shaderc/shaderc.hpp>
#include <string.h>

using namespace  zr::core;

void ShaderModule::load_shader_from_file(const char *file_path) {

    std::string content = utils::FileHelper().load_content(file_path);
    VkShaderModuleCreateInfo shader_module_ci;
    
    shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_ci.pNext = nullptr;
    shader_module_ci.flags = 0;
    shader_module_ci.codeSize = content.size();
    shader_module_ci.pCode = (const uint32_t*) content.c_str();
    
    CALL_VK(vkCreateShaderModule(_device->get_device(), &shader_module_ci, nullptr, &_vk_shader_module));
}

ShaderModule::ShaderModule(std::shared_ptr<Device> device, const char *file_path, bool use_bin,
                           std::vector<std::string> defines) : _device(device) {
    if (use_bin) {
        load_shader_from_file(file_path);
    }
    else {
        uint32_t file_name_start_index = 0;
        uint32_t idx = 0;
        while(file_path[idx]){
            if (file_path[idx] == '/') {
                file_name_start_index = idx + 1;
            }
            ++idx;
        }
        char base_dir[256] = {0};
        memcpy(base_dir, file_path, file_name_start_index * sizeof(char));
        char file_name[256] = {0};
        memcpy(file_name, file_path + file_name_start_index, (strlen(file_path) - file_name_start_index) * sizeof(char));
        create_shader_from_file(base_dir, file_name, defines);
    }
}

ShaderModule::~ShaderModule() {
    vkDestroyShaderModule(_device->get_device(), _vk_shader_module, nullptr);
}

void ShaderModule::create_shader_from_file(const char* base_dir, const char *file_path, std::vector<std::string> defines) {

    utils::CodeGenerator code_generator;
    std::stringstream  ss;
    code_generator.execute(ss, base_dir, file_path, defines);
    std::string code = ss.str();

    shaderc::Compiler compiler;
    shaderc_shader_kind kind = shaderc_vertex_shader;
    uint32_t file_path_len = strlen(file_path);
    if (strcmp(file_path + (file_path_len - 4), "frag") == 0) {
        kind = shaderc_fragment_shader;
    }

    shaderc::CompilationResult<uint32_t> ret = compiler.CompileGlslToSpv(code.c_str(), code.size(), kind, file_path);
    uint32_t error_num = ret.GetNumErrors();
    std::string error_msg = ret.GetErrorMessage();

#ifndef PLATFORM_ANDROID
    if (error_num > 0) {
        LOGD(code.c_str());
        LOGD(error_msg.c_str());
    }
#else
    if (error_num > 0) {
        LOGD("%s", code.c_str());
        LOGD("%s", error_msg.c_str());
    }    
#endif

    assert(ret.GetNumErrors() == 0);
    if (ret.GetNumErrors() > 0) {
        return;
    }
    const uint32_t * begin = ret.cbegin();
    const uint32_t * end = ret.cend();
    uint32_t code_size = (end - begin) * sizeof(uint32_t);
    VkShaderModuleCreateInfo shader_module_ci;
    
    shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_ci.pNext = nullptr;
    shader_module_ci.flags = 0;
    shader_module_ci.codeSize = code_size;
    shader_module_ci.pCode = (const uint32_t*) begin;
    
    CALL_VK(vkCreateShaderModule(_device->get_device(), &shader_module_ci, nullptr, &_vk_shader_module));
}
